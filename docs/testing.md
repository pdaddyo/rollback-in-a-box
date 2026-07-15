# Testing

Run the full local suite:

```sh
./tests/run_all.sh
```

The runner imports the project before each batch and treats GDScript parse
errors as failures. The suite covers:

- `test_smoke.gd`: extension registration, primitive convenience methods,
  stepping, snapshots, hashes, restore, and deterministic resimulation.
- `test_raw_api.gd`: generated catalog coverage, all six runtime shape families,
  all nine joint families, family-specific mutation, world stepping, bounds,
  and a world query.
- `test_determinism.gd`: independent worlds produce identical hashes, then one
  restores an older snapshot and reproduces the recorded hash sequence.
- `test_session_loopback.gd`: two network sessions converge with clean delivery
  and with deterministic latency, jitter, 12 percent loss, prediction, and
  uneven confirmation.
- `test_replay.gd`: recording, file validation, replay load, forward/backward
  seek, shape enumeration, transform streaming, and divergence checks.

Edge-case suites exercise guard paths, degenerate inputs, and boundary
conditions:

- `test_edge_raw_marshaling.gd`: generated bridge marshaling — bad struct
  names, wrong field types, array-length validation, unknown enum values, and
  world queries with null callbacks.
- `test_edge_ids_shapes.gd`: id lifecycle — stale ids after destroy, double
  destroy, wrong-family mutations, and validity checks across reinit.
- `test_edge_world_snapshots.gd`: snapshot slot bounds, zero/negative slot
  counts, restore before step, restore across reinit, and cross-world restore.
- `test_edge_world_hashing.gd`: hash stability across restore, hash advance on
  quiescent worlds, empty-world hashing, and extreme time steps.
- `test_edge_session.gd`: session netcode — duplicate and out-of-order input
  delivery, frame-boundary conditions, stall behavior, and configure guard
  rails after start.
- `test_edge_replay.gd`: empty recordings, out-of-range seeks, corrupt and
  truncated files, and use-before-load.

Also run the generated coverage check:

```sh
python3 tools/generate_bindings.py --check
```

CI builds debug and release libraries on macOS, Windows, and Linux. The macOS
job runs the headless Godot suite; macOS and Linux verify generated API coverage.
