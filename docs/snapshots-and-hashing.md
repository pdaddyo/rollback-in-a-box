# Snapshots and hashing

`Box3DRollbackWorld` restores Box3D in place through the same internal snapshot
machinery used by Box3D recording/replay. Snapshot slots are reusable buffers;
after warm-up, repeatedly saving the same slot does not allocate a new buffer.

The world stores the current frame, its convenience-layer body table, and body
metadata beside each Box3D snapshot. `state_hash()` combines Box3D's canonical
world hash with normalized convenience state. Per-process world indices are
excluded so independent peers can compare hashes.

Box3D snapshots do not contain your gameplay state. A game simulation object
should save and load all deterministic side-state in the same slot:

```gdscript
func rollback_save_state(slot: int) -> bool:
  if not physics.save_state(slot):
    return false
  game_slots[slot] = {
    "health": health.duplicate(),
    "rng": rng_state,
    "spawn_counter": spawn_counter,
  }
  return true
```

Include in side-state and hashing:

- entity identity and live/dead state
- health, cooldowns, timers, and mode flags
- deterministic RNG state
- spawn counters and input edge-detection state
- any table that maps game entities to Box3D ids

Exclude presentation-only interpolation, particles, camera state, audio, and UI.

Objects created after a saved frame disappear when that frame is restored.
Restore the corresponding game handle table with the same slot so stale ids
cannot be used after rollback.
