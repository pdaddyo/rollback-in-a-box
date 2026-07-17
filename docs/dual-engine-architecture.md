# Dual-engine architecture

This repo supports two engines from one codebase. The rollback and physics logic
lives in an **engine-neutral core** (`/core`) that depends only on Box3D. Each
engine is a thin adapter over that core:

- **Godot** — the GDExtension in `/gdext` (classes `Box3DRollbackWorld`,
  `Box3DRollbackSession`, plus the Godot-only `Box3DRaw`/`Box3DReplay`).
- **Unreal 5.8** — the plugin in `/unreal/Box3DRollback`
  (`UBox3DRollbackWorld`, `UBox3DRollbackSession`, `IBox3DRollbackSimulation`).

There is exactly one copy of the rollback logic. The adapters marshal engine
types to and from the neutral core; they contain no netcode or physics.

```
                     ┌────────────────────────────┐
                     │  Box3D (vendored, pinned)   │
                     └──────────────┬──────────────┘
                                    │
                     ┌──────────────┴──────────────┐
                     │   /core  engine-neutral      │
                     │   rb::RollbackWorld          │
                     │   rb::RollbackSession        │
                     │   rb::IRollbackSimulation    │
                     │   rollback_shim.c (snapshot) │
                     └───────┬──────────────┬───────┘
                             │              │
              ┌──────────────┴───┐     ┌────┴──────────────────┐
              │  /gdext  (Godot) │     │  /unreal (UE 5.8)     │
              │  Box3DRollback*  │     │  UBox3DRollback*      │
              │  + Box3DRaw      │     │  + IBox3DRollback...  │
              │  + Box3DReplay   │     │                       │
              └──────────────────┘     └───────────────────────┘
```

## The engine-neutral core (`/core`)

Public headers live in `core/include/box3d_rollback/`. No godot-cpp, no Unreal,
no `Variant`/`FVector` — only plain C++, `std::vector`, and (where a class wraps
a physics world) Box3D types.

### `rb::IRollbackSimulation`

The simulation contract the session drives. Historically the Godot session
called seven stringly-typed methods on a GDScript `Object` (`sim->call(
"rollback_save_state", …)`); that is now a C++ interface:

```cpp
struct IRollbackSimulation {
    virtual bool     is_alive() const { return true; }
    virtual bool     has_world() const = 0;
    virtual int      get_input_count() const = 0;
    virtual void     init_snapshots(int slot_count) = 0;
    virtual bool     save_state(int slot) = 0;
    virtual bool     load_state(int slot) = 0;
    virtual uint64_t get_state_hash() const = 0;
    virtual void     step_frame(const int64_t* inputs, int count) = 0;
    virtual bool     supports_rollback_begin() const { return false; }
    virtual void     rollback_begin(int64_t target, int window, int mask) {}
};
```

`rb::ISessionObserver` carries the two edge events (`on_desync`,
`on_peer_incompatible`) the session used to emit as Godot signals.

### `rb::RollbackWorld`

The deterministic Box3D world wrapper (bodies, fixed step, reusable snapshot
slots, FNV state hash, transforms, recording, partial-resim scoping). It
**implements `IRollbackSimulation` directly**, so a pure-C++ or Unreal caller
can hand a world straight to a session. Its public API takes plain PODs
(`rb::Vec3`, `rb::Transform`, `std::vector<float>`, `rb::RollbackScope`) instead
of `Vector3`/`Transform3D`/`PackedFloat32Array`/`Dictionary`.

### `rb::RollbackSession`

Prediction, rollback/resimulation, input resend/ack, confirmed-frame hash
exchange, desync detection, and frame-advantage throttling — moved verbatim from
the Godot class. Packets are `std::vector<uint8_t>`; the wire format, packet
magic/version, and `write_le`/`read_le` layout are byte-for-byte unchanged, so
peers on the old Godot build and the new one interoperate.

### `rollback_shim.c`

The C shim over Box3D's internal snapshot machinery, moved unchanged from
`/gdext/src`. Still C, so Box3D's internal C headers never enter a C++
translation unit. It also owns `b3r_determinism_fingerprint()`.

## The Godot adapter (`/gdext`)

`Box3DRollbackWorld`/`Box3DRollbackSession` are now thin `GDCLASS` wrappers that
own an `rb::` core object by value and marshal Godot types. The bound method
names, signatures, and the `desync_detected`/`peer_incompatible` signals are
unchanged — existing GDScript, tests, and the `rollback_*` sim interface all
keep working.

Two small bridges connect the neutral core back to Godot's dynamic world:

- **`GodotSimBridge : rb::IRollbackSimulation`** — forwards each interface call
  to a GDScript `Object` via `call("rollback_…")`, resolving the object through
  a weak `ObjectID` so a freed sim reports `is_alive() == false`. Games still
  implement the seven `rollback_*` methods in GDScript exactly as before.
- **`SignalObserver : rb::ISessionObserver`** — re-emits core events as Godot
  signals.

`Box3DRaw` (the 700-function generated Variant bridge) and `Box3DReplay` (the
replay viewer) stay Godot-only and unchanged.

## The Unreal adapter (`/unreal/Box3DRollback`)

A runtime UE 5.8 plugin. `UBox3DRollbackWorld` and `UBox3DRollbackSession` are
`UObject`s that hold the neutral core behind a `TUniquePtr` (PIMPL), so no Box3D
or neutral headers leak into the reflected `UCLASS` surface. Methods are
`UFUNCTION(BlueprintCallable)`; core events surface as `BlueprintAssignable`
dynamic multicast delegates.

`IBox3DRollbackSimulation` is a `BlueprintNativeEvent` UINTERFACE mirroring
`rb::IRollbackSimulation`. A native bridge forwards the seven calls to any
implementing UObject via the UHT `Execute_*` thunks — the Unreal analogue of
`GodotSimBridge`. Games that add no custom step logic skip the interface and
bind a world directly (`Session->SetSimulationWorld(World)`), because
`rb::RollbackWorld` already satisfies the interface.

UBT does not use CMake. The core and Box3D are compiled once as static libraries
by `unreal/build_thirdparty.sh` (which drives `unreal/thirdparty/CMakeLists.txt`)
and linked via `Box3DRollback.Build.cs`. See the plugin README for setup.

## What is not ported, and why

- **`Box3DRaw` / the 700-function facade** — its purpose is to give a scripting
  layer (GDScript) access to raw Box3D. Unreal C++ includes Box3D headers and
  calls the C API directly, so an equivalent bridge would be redundant. The
  generator remains Godot-targeted.
- **`Box3DReplay`** — a rendering/debug tool. Playback and meshing belong in the
  game/engine layer; Unreal games render replays with their own actors.

Both could be added later (e.g. a generated Blueprint facade) without touching
the core.

## Determinism across engines

The determinism contract (`docs/determinism-contract.md`) is unchanged, and the
FP flags (`-ffp-contract=off` / `/fp:precise`) are applied to the neutral core
target itself, plus Box3D, in every build path (Godot, Unreal thirdparty, and
standalone). The build fingerprint (Box3D version, SIMD flavor/width, float eval
mode, snapshot struct layouts, and a hashed 90-frame probe) gates packets: peers
with different fingerprints are rejected at the packet layer.

Matching fingerprints are what make peers compatible — always **within** an
engine, and **across** Godot and Unreal only when the core is compiled
equivalently. Cross-engine play is therefore a *tested* configuration, verified
by comparing `get_build_fingerprint()` on both sides, not an automatic guarantee.

## Building

Godot GDExtension:

```sh
./gdext/setup_deps.sh
cmake -S gdext -B gdext/build-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DGODOTCPP_TARGET=template_debug
cmake --build gdext/build-debug
```

Unreal plugin:

```sh
./gdext/setup_deps.sh          # shared: clones the pinned Box3D
./unreal/build_thirdparty.sh   # builds box3d + box3d_rollback_neutral static libs
# then enable unreal/Box3DRollback in a UE 5.8 project and build.
```
