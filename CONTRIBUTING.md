# Contributing

## Setup

```sh
./gdext/setup_deps.sh
cmake -S gdext -B gdext/build-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DGODOTCPP_TARGET=template_debug
cmake --build gdext/build-debug
./tests/run_all.sh
```

## Pull requests

- Keep the raw binding mechanical and game-agnostic.
- Add focused tests for behavioral changes.
- Run `python3 tools/generate_bindings.py --check` and `git diff --check`.
- Do not edit files marked as generated. Change the generator and regenerate.
- Document changes to determinism, packet compatibility, ownership, or snapshot
  behavior explicitly.

## Compatibility

The packet version, snapshot implementation, Box3D commit, compiler floating
point behavior, worker count, fixed timestep, and substep count can affect
determinism. Changes to any of these need cross-peer tests and a clear release
note.
