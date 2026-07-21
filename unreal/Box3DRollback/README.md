# Box3D Rollback — Unreal Engine 5.8 plugin

Deterministic network rollback physics on [Box3D](https://github.com/erincatto/box3d),
for Unreal Engine 5.8. This plugin is a **thin adapter** over the engine-neutral
rollback core in the repo's `/core` directory — the exact same C++/C that the
Godot GDExtension compiles. There is no second copy of the rollback logic and no
dependency on Godot. See `docs/dual-engine-architecture.md` for the full picture.

## What you get

- `UBox3DRollbackWorld` — a deterministic Box3D world: create bodies, fixed
  stepping, reusable snapshots, state hashing, transforms, and recording. It
  also implements `IBox3DRollbackSimulation`, so a session can drive it directly.
- `UBox3DRollbackSession` — prediction, rollback/resimulation, input
  resend/ack, confirmed-frame hash exchange, desync detection, and
  frame-advantage throttling. Packets are opaque `TArray<uint8>` you forward
  over any transport. Desync / incompatible-peer events surface as Blueprint
  `OnDesyncDetected` / `OnPeerIncompatible` delegates.
- `IBox3DRollbackSimulation` — a Blueprint-implementable interface for games
  that add custom per-frame logic around the world.

The raw 700-function Box3D binding and the replay viewer are **not** ported:
Unreal C++ can call Box3D directly, and rendering belongs in the game. See the
architecture doc for the rationale.

Box3D has process-global state, so modular builds must keep one live copy. The
prebuilt static libraries are a private dependency of this plugin module. On
Windows, raw consumers import the plugin's re-exported B3_API. On other targets,
engine-neutral consumers call the needed functions through the versioned table
in `Box3DRollbackRawApi.h`; directly linking the static library into another
module would create an isolated `b3_worlds` array.

## Prerequisites

1. Fetch the pinned Box3D + build the shared static libraries (from the repo root):

   ```sh
   ./gdext/setup_deps.sh        # once — clones the pinned Box3D
   ./unreal/build_thirdparty.sh # builds box3d + box3d_rollback_neutral static libs
   ```

   On Windows, run these from a Developer Command Prompt (so MSVC is on PATH) via
   Git Bash, or run the equivalent CMake commands:

   ```sh
   cmake -S unreal/thirdparty -B unreal/thirdparty/build -DCMAKE_BUILD_TYPE=Release
   cmake --build unreal/thirdparty/build --config Release
   # then copy the .lib files into
   # unreal/Box3DRollback/Source/ThirdParty/Box3DRollbackLibrary/lib/Win64/
   ```

2. Copy or symlink `unreal/Box3DRollback/` into your UE project's `Plugins/`
   folder (or the engine's `Plugins/Marketplace/`), then enable **Box3D Rollback**
   in Edit → Plugins and rebuild.

## Usage (C++)

```cpp
// Build a world.
UBox3DRollbackWorld* World = NewObject<UBox3DRollbackWorld>(this);
World->SetInputCount(2);
World->CreateWorld();
World->AddStaticBox(FVector(0, -1, 0), FVector(20, 1, 20));
const int32 Player0 = World->AddDynamicSphere(FVector(0, 5, 0), 0.5f);

// Drive it with a session (the world is its own simulation).
UBox3DRollbackSession* Session = NewObject<UBox3DRollbackSession>(this);
Session->SetSimulationWorld(World);
Session->Configure(/*LocalPlayer*/ 0, /*NumPlayers*/ 2);
Session->Start();

Session->OnDesyncDetected.AddDynamic(this, &AMyPawn::HandleDesync);

// Each fixed tick:
if (Session->Tick(LocalInputBits))
{
    const TArray<uint8> Packet = Session->GetPacket();
    MyTransport->SendUnreliable(Packet);      // your netcode
}
// On receive:
Session->IngestPacket(ReceivedBytes);
```

For games that add rules on top of the physics, implement
`IBox3DRollbackSimulation` on your own UObject (wrapping a `UBox3DRollbackWorld`
and applying inputs before `StepFrame`) and bind it with
`Session->SetSimulationInterface(MySim)`.

## Determinism across engines

Rollback correctness needs bit-identical resimulation across peers. The core
carries a **determinism fingerprint** (Box3D version, SIMD flavor/width, float
eval mode, snapshot struct layouts, and a hashed 90-frame simulation probe).
`UBox3DRollbackSession::GetBuildFingerprint()` exposes it; the session rejects
packets from peers whose fingerprint differs, and games should compare
fingerprints during matchmaking.

Two peers can safely play only if their fingerprints match — this holds
**within** an engine build and, when the underlying core is compiled the same
way, **across** Godot and Unreal. Do not assume a Godot client and an Unreal
client interoperate until their fingerprints are confirmed equal; treat
cross-engine play as a tested configuration, not a given.

> Status: the module **compiles and links under UE 5.8** — UnrealHeaderTool
> processes the reflected types and `UnrealEditor-Box3DRollback.dll` links the
> `box3d` + `box3d_rollback_neutral` static libs (verified via a host project
> that also consumes the Blueprint API from a separate module). The neutral core
> it wraps is separately unit-exercised (an input-driven two-peer rollback
> loopback: prediction, resimulation, and confirmed-frame hash agreement). The
> in-editor runtime path has a compile-smoke consumer but has not yet been driven
> live in PIE — do that against your project before shipping.
