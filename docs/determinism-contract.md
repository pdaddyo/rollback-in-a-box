# Determinism Contract

Rollback only works when every peer can resimulate the same frames and produce
the same state hashes.

Required:

- Same addon build on all peers.
- Same Box3D commit on all peers.
- Same world construction order.
- Same fixed timestep and substep count.
- Same worker count.
- Same simulation-affecting game-side state restored with each snapshot.
- No peer-local randomness during simulation.
- No use of compiler flags that alter floating point semantics, such as
  `-ffast-math`.

The addon builds sim code with `-ffp-contract=off` on clang/gcc and `/fp:precise`
on MSVC.

Box3D's public API does not expose rollback snapshots. This addon vendors Box3D
and uses the same internal snapshot machinery as Box3D's recording/replay
system through `rollback_shim.c`.

Snapshots cover Box3D state. Your game must also snapshot any deterministic
state outside Box3D: entity tables, cooldowns, health, RNG state, spawn
counters, input edge state, and similar data.
