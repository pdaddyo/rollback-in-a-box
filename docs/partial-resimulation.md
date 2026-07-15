# Partial resimulation

Full-world rollback is the correctness baseline: restore a snapshot, resimulate
every frame with corrected inputs. This page describes what the extension does
to scope that work, what it deliberately does not do, and why.

## What is scoped today

**Sleeping islands are already skipped by the solver.** Box3D solves only awake
islands. In a world where most content is settled debris, resimulating a frame
costs solver time proportional to the awake bodies, not the world size.
Snapshot save/restore remains whole-world but is memcpy-bound.
`tests/test_partial_resim.gd` verifies this end to end: an 81-body world with
78 sleeping debris bodies converges bit-for-bit across peers through hundreds
of rollback frames while only ~3 bodies are awake.

**The affected set of a rollback is computed and reported.** Games declare
which bodies each player's input directly influences:

```gdscript
world.set_player_bodies(0, PackedInt64Array([player_body]))
```

When a rollback happens, the session calls the optional simulation method

```gdscript
func rollback_begin(target_frame: int, window_frames: int, players_mask: int) -> void
```

with the mask of players whose inputs were mispredicted.
`Box3DRollbackWorld.rollback_begin()` computes the affected set — the closure
of the mispredicted players' declared bodies over contacts, joints, and
swept-AABB proximity for the resimulated window — and records it. Static
bodies do not propagate the closure (the ground does not connect everything to
everything). `get_last_rollback_scope()` returns the telemetry:

```gdscript
{ valid, target_frame, window, mispredicted_mask,
  affected_bodies, awake_bodies, total_bodies }
```

`compute_affected_bodies(players_mask, window_frames)` exposes the same
closure directly and is deterministic across peers (compare by handle; raw ids
embed a per-process world index).

## What is deliberately not done: skipping awake islands

Rolling back only player-touched islands — leaving awake-but-unaffected
islands at their current state instead of resimulating them — requires
transplanting those islands' complete internal solver state between snapshot
eras. The affected set's trajectory must be recomputed; the unaffected awake
islands' frame-N state must come from somewhere, and the only bit-exact source
is the frame-N snapshot image. Splicing per-island state between snapshot
images means merging solver sets, constraint-graph membership, contact
warm-start impulses, per-body sleep timers, deferred island-split scheduling,
and broadphase proxy bounds. Getting any of these wrong does not fail loudly:
it produces a state that differs from a full-resimulation peer by a few float
bits, which surfaces as a confirmed-hash desync hundreds of frames later.

Freezing unaffected awake islands during resimulation is not a shortcut: a
frozen island keeps its restore-era position, but its true frame-N position
differs because it was moving. Sleep-based freezing is only exact for islands
that are asleep for the whole window — and the solver already skips those.

The affected-set machinery above is the deterministic foundation for a future
island transplant implemented inside the snapshot machinery. Until then, the
correctness contract stays simple: every resimulated frame is a full-world
step, and every peer's confirmed hash is comparable.
