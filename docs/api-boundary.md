# API boundary

The extension has four layers. Dependencies point downward; game rules stay
outside all four.

## Raw Box3D

`Box3DRaw` is the native dispatcher and `Box3D` is its generated, named
GDScript facade. The generator reads Box3D's pinned public headers and covers:

- worlds, bodies, shapes, contacts, and joints
- sphere, capsule, hull, mesh, height-field, and compound geometry
- all nine joint families and every family-specific tuning call
- queries, events, collision helpers, mass helpers, and dynamic trees
- recording, replay, profiles, counters, debug data, and public math helpers

This layer preserves the C model:

- Box3D ids are packed into Godot `int` values.
- Value structs are Dictionaries, native Godot math values, or exact bytes.
- Pointer parameters use `Box3DBuffer` or an address returned by Box3D.
- Box3D-created resources are destroyed with their matching Box3D function.

Raw ids are not game entity ids. Do not serialize native addresses or send
them over the network.

## Deterministic world

`Box3DRollbackWorld` owns one Box3D world, a fixed frame counter, snapshot
slots, state hashing, and a small convenience API. It implements the simulation
contract expected by `Box3DRollbackSession`.

Snapshots restore the Box3D world in place. Box3D ids for objects that existed
in the snapshot remain usable after restore. Objects created after that
snapshot no longer exist after restore; callers must restore their own handle
tables and gameplay side-state at the same time.

The convenience methods such as `add_dynamic_box()` are sugar. They do not
limit the raw API.

## Network rollback

`Box3DRollbackSession` owns deterministic input exchange:

- input delay and sticky prediction
- snapshot selection, rollback, and resimulation
- redundant input history and acknowledgements
- confirmed-frame hash exchange and desync detection
- frame-advantage throttling

It depends only on a seven-method simulation interface. It does not depend on
`Box3DRollbackWorld`, sockets, Godot multiplayer, matchmaking, or game rules.

## Transport and game

The application owns:

- UDP, ENet, WebRTC, Steam, relay, or another byte transport
- player assignment, session negotiation, seeds, and content version checks
- deterministic gameplay state and RNG
- rendering, audio, UI, and interpolation

Transport code forwards opaque `PackedByteArray` packets. A game simulation
wrapper snapshots its side-state alongside the physics world and combines that
state into the hash.

## Dependency rule

The raw binding never calls rollback code. The snapshot layer never calls the
network session. The network session calls only the simulation interface. No
extension layer contains game-specific entities, weapons, vehicles, terrain,
scoring, or rendering metadata.

The implementation can run as its own GDExtension or be embedded through the
`box3d_rollback_core` CMake object target. Embedded hosts call
`register_box3d_rollback_classes()` from their initializer; this changes only
linkage, not the API or ownership boundary.
