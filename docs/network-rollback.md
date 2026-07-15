# Network rollback

`Box3DRollbackSession` owns input prediction, rollback, resimulation, packet
encoding, acknowledgements, confirmed hashes, desync detection, and pacing. It
does not open a socket and does not depend on Godot's multiplayer API.

## Simulation interface

The session drives any Godot object implementing:

```gdscript
func rollback_has_world() -> bool
func rollback_get_input_count() -> int
func rollback_init_snapshots(slot_count: int) -> void
func rollback_save_state(slot: int) -> bool
func rollback_load_state(slot: int) -> bool
func rollback_state_hash() -> int
func rollback_step_frame(inputs: PackedInt64Array) -> void
```

`Box3DRollbackWorld` implements this interface for physics-only simulations. A
real game normally wraps it and saves gameplay side-state in the same slots.

## Session setup

Every peer must agree on player count, player-to-index assignment, input delay,
maximum prediction, world seed/content, timestep, substeps, worker count, and
build compatibility before `start()`.

```gdscript
var session := Box3DRollbackSession.new()
session.set_simulation(simulation)
session.configure(local_player_index, player_count, 2, 8)
session.desync_detected.connect(_on_desync_detected)
session.start()
```

The current implementation supports one to four players, keeps a 128-frame
input/hash ring, and uses 64 reusable snapshot slots. Maximum prediction must
stay below the snapshot horizon.

## Tick and transport loop

```gdscript
for packet in transport.poll_packets():
  session.ingest_packet(packet)

var advanced := session.tick(sample_local_input())
var packet := session.get_packet()
if not packet.is_empty():
  transport.send(packet)

if not advanced and session.is_stalled():
  # Keep polling and sending. The peer is waiting for confirmed input or pacing.
  pass
```

`tick()` returns `true` only when one simulation frame advances. It can return
`false` when prediction reaches its configured limit or frame-advantage pacing
briefly throttles the faster peer. Packet exchange should continue while
stalled.

## Prediction and correction

Missing remote input uses sticky prediction: the last confirmed value for that
player. When a late packet corrects an already simulated frame, the session
loads the snapshot immediately before that frame and resimulates to the current
frame. `get_last_rollback_depth()` and `get_total_rollback_frames()` expose the
cost.

## Confirmation and desyncs

Packets include a state hash only for a frame that is confirmed for every
player and has no pending correction. A peer compares that hash once its own
same frame is safe. A mismatch sets `is_desynced()`, records
`get_desync_frame()`, and emits `desync_detected(frame)`.

Treat a desync as terminal for the match unless the application has an explicit
authoritative resynchronization protocol. This extension detects divergence;
it does not choose an authority or transfer full game state.

## Packet compatibility

Packets are little-endian, begin with magic `D3RB`, and currently use protocol
version 1. The payload carries sender identity, pacing data, current frame,
acknowledgement, safe-frame hash, and up to 64 redundant input values. Packet
contents are opaque to the application. Invalid version, sender, size, count,
or far-future frame data is discarded.
