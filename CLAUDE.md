# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Deterministic network rollback physics on top of Box3D (vendored at a pinned revision), for **two engines from one core**. The rollback/physics logic is an engine-neutral C++/C library in `core/` (namespace `rb::`, depends only on Box3D). Each engine is a thin adapter over it:

- `gdext/` — the Godot 4 GDExtension (adapters + the generated raw `Box3D` facade + replay viewer); packaged addon in `addons/box3d_rollback/`; headless GDScript tests in `tests/`.
- `unreal/Box3DRollback/` — the Unreal Engine 5.8 plugin (`UBox3DRollback*` UObject adapters).

See `docs/dual-engine-architecture.md`. When changing rollback or physics behavior, edit `core/` (the shared logic) — not an adapter — and keep both adapters' marshalling in sync.

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

# Unreal: build the shared core + box3d as static libs, then enable the plugin
./unreal/build_thirdparty.sh   # stages libs for unreal/Box3DRollback/Source/ThirdParty
```

The Godot build writes the extension library into `addons/box3d_rollback/bin/`, which the Godot project loads via `box3d_rollback.gdextension`. The Unreal plugin (`unreal/Box3DRollback/`) links the same `core/` sources as prebuilt static libraries via `Box3DRollback.Build.cs`.

## Architecture

Four layers with strict downward-only dependencies (see `docs/api-boundary.md` and `docs/dual-engine-architecture.md`). Game rules, rendering, and byte transport stay outside all of them. Layers 2-4 are **engine-neutral** and live in `core/` (namespace `rb::`); the Godot and Unreal classes are thin adapters that marshal engine types to/from the core.

1. **Raw Box3D binding** (Godot-only) — `Box3DRaw` (C++ dispatcher in `gdext/src/box3d_raw.cpp`) plus the generated GDScript facade `Box3D` (`addons/box3d_rollback/box3d.gd`). Covers all 700 public Box3D functions with exact C names. Box3D ids are packed into Godot ints; structs are Dictionaries/`Box3DBuffer` byte buffers. Not ported to Unreal (UE C++ calls Box3D directly).
2. **Deterministic world** — `rb::RollbackWorld` (`core/src/rb_world.cpp`): owns one Box3D world, fixed frame counter, reusable snapshot slots, and state hashing; implements the simulation interface directly. Adapted by `Box3DRollbackWorld` (Godot) and `UBox3DRollbackWorld` (Unreal). Convenience methods (`add_dynamic_box()` etc.) are sugar, not the API boundary.
3. **Rollback networking** — `rb::RollbackSession` (`core/src/rb_session.cpp`): prediction, rollback/resimulation, input resend/acking, confirmed-frame hashes, desync detection, frame-advantage throttling. Depends only on the `rb::IRollbackSimulation` seven-method interface (`save_state`, `load_state`, `step_frame`, ...) — not on `rb::RollbackWorld`, sockets, or engine multiplayer. Packets are opaque byte buffers the game forwards over any transport. In Godot the game still implements the `rollback_*` methods on a GDScript object; `GodotSimBridge` forwards the interface to them via reflection.
4. **Snapshot shim** — `core/src/rollback_shim.c` / `core/include/box3d_rollback/rollback_shim.h`: a C shim over Box3D's *internal* snapshot machinery (not public API — this is why Box3D is vendored). Kept in C so internal C headers never enter a C++ translation unit. Lifetime rule: destroy the world before the rollback context.

`Box3DReplay` (`gdext/src/box3d_replay.cpp`, Godot-only) handles recording/replay. `gdext/src/register_types.cpp` registers everything for the standalone extension; the `box3d_rollback_core` CMake OBJECT target lets host projects embed the classes (call `register_box3d_rollback_classes()`), and the neutral logic is the `box3d_rollback_neutral` static library (`core/CMakeLists.txt`) that both engines link.

### Code generation

`tools/generate_bindings.py` parses Box3D's pinned public headers with clang and emits:
- `gdext/src/generated/*.inc` (catalog, function dispatch, struct marshalling)
- `addons/box3d_rollback/box3d.gd` (the `Box3D` facade)
- `docs/api-reference.md` and `api/box3d-api.json`

Never hand-edit generated files (marked "Do not edit") — change the generator and regenerate. CI runs `--check` so coverage can't silently drift when the pinned Box3D revision changes.

## Determinism constraints

Rollback correctness depends on bit-identical resimulation across peers (see `docs/determinism-contract.md`):

- Sim code is built with `-ffp-contract=off` (clang/gcc) or `/fp:precise` (MSVC) — set on the neutral core target in `core/CMakeLists.txt` and in every build path (`gdext/CMakeLists.txt`, `unreal/thirdparty/CMakeLists.txt`). Never add flags that alter FP semantics (e.g. `-ffast-math`).
- Packet version, snapshot format, the pinned Box3D commit (`gdext/setup_deps.sh`), worker count, fixed timestep, and substep count all affect determinism or wire compatibility. Changes to any of these need cross-peer tests and an explicit release note.
- Document changes to determinism, packet compatibility, ownership, or snapshot behavior explicitly in PRs.

## Conventions

- Keep the raw binding mechanical and game-agnostic; no extension layer may contain game-specific concepts (entities, weapons, scoring, rendering metadata).
- Run `python3 tools/generate_bindings.py --check` and `git diff --check` before submitting.
- Dependencies are pinned: `gdext/setup_deps.sh` holds the Box3D SHA and godot-cpp tag.
