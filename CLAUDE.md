# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A Godot 4 GDExtension providing deterministic network rollback physics on top of Box3D (vendored at a pinned revision). C++/C native code lives in `gdext/`, the packaged addon in `addons/box3d_rollback/`, and headless GDScript tests in `tests/`.

## Commands

```sh
# One-time: clone pinned box3d + godot-cpp into gdext/extern/
./gdext/setup_deps.sh

# Build (debug)
cmake -S gdext -B gdext/build-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DGODOTCPP_TARGET=template_debug
cmake --build gdext/build-debug

# Build (release)
cmake -S gdext -B gdext/build-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DGODOTCPP_TARGET=template_release
cmake --build gdext/build-release

# Run all tests (requires a Godot binary; defaults to /Applications/Godot.app/Contents/MacOS/Godot, override with GODOT=...)
./tests/run_all.sh

# Run a single test
GODOT=... ; "$GODOT" --headless -s tests/test_determinism.gd
# Tests signal failure via nonzero exit OR "SCRIPT ERROR:" / "Parse Error:" / "FAIL:" in output.

# Verify generated bindings are in sync (run before PRs)
python3 tools/generate_bindings.py --check

# Regenerate bindings (requires clang; parses Box3D headers via AST dump)
python3 tools/generate_bindings.py
```

The build writes the extension library into `addons/box3d_rollback/bin/`, which the Godot project loads via `box3d_rollback.gdextension`.

## Architecture

Four layers with strict downward-only dependencies (see `docs/api-boundary.md`). Game rules, rendering, and byte transport stay outside all of them.

1. **Raw Box3D binding** — `Box3DRaw` (C++ dispatcher in `gdext/src/box3d_raw.cpp`) plus the generated GDScript facade `Box3D` (`addons/box3d_rollback/box3d.gd`). Covers all 700 public Box3D functions with exact C names. Box3D ids are packed into Godot ints; structs are Dictionaries/`Box3DBuffer` byte buffers. This layer is mechanical and game-agnostic; it never calls rollback code.
2. **Deterministic world** — `Box3DRollbackWorld` (`gdext/src/box3d_rollback_world.cpp`): owns one Box3D world, fixed frame counter, reusable snapshot slots, and state hashing. Convenience methods (`add_dynamic_box()` etc.) are sugar, not the API boundary.
3. **Rollback networking** — `Box3DRollbackSession` (`gdext/src/box3d_rollback_session.cpp`): prediction, rollback/resimulation, input resend/acking, confirmed-frame hashes, desync detection, frame-advantage throttling. Depends only on a seven-method simulation interface (`rollback_save_state`, `rollback_load_state`, `rollback_step_frame`, ...) — not on `Box3DRollbackWorld`, sockets, or Godot multiplayer. Packets are opaque `PackedByteArray`s the game forwards over any transport.
4. **Snapshot shim** — `rollback_shim.c`/`.h`: a C shim over Box3D's *internal* snapshot machinery (not public API — this is why Box3D is vendored). Kept in C so internal C headers never enter a C++ translation unit. Lifetime rule: destroy the world before the rollback context.

`Box3DReplay` (`box3d_replay.cpp`) handles recording/replay. `register_types.cpp` registers everything for the standalone extension; the `box3d_rollback_core` CMake OBJECT target lets host projects embed the classes instead (call `register_box3d_rollback_classes()`).

### Code generation

`tools/generate_bindings.py` parses Box3D's pinned public headers with clang and emits:
- `gdext/src/generated/*.inc` (catalog, function dispatch, struct marshalling)
- `addons/box3d_rollback/box3d.gd` (the `Box3D` facade)
- `docs/api-reference.md` and `api/box3d-api.json`

Never hand-edit generated files (marked "Do not edit") — change the generator and regenerate. CI runs `--check` so coverage can't silently drift when the pinned Box3D revision changes.

## Determinism constraints

Rollback correctness depends on bit-identical resimulation across peers (see `docs/determinism-contract.md`):

- Sim code is built with `-ffp-contract=off` (clang/gcc) or `/fp:precise` (MSVC) — set in `gdext/CMakeLists.txt`. Never add flags that alter FP semantics (e.g. `-ffast-math`).
- Packet version, snapshot format, the pinned Box3D commit (`gdext/setup_deps.sh`), worker count, fixed timestep, and substep count all affect determinism or wire compatibility. Changes to any of these need cross-peer tests and an explicit release note.
- Document changes to determinism, packet compatibility, ownership, or snapshot behavior explicitly in PRs.

## Conventions

- Keep the raw binding mechanical and game-agnostic; no extension layer may contain game-specific concepts (entities, weapons, scoring, rendering metadata).
- Run `python3 tools/generate_bindings.py --check` and `git diff --check` before submitting.
- Dependencies are pinned: `gdext/setup_deps.sh` holds the Box3D SHA and godot-cpp tag.
