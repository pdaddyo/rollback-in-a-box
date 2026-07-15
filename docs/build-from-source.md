# Build From Source

## Requirements

- Godot 4.5 or newer
- CMake 3.22 or newer
- Ninja
- Python 3
- Git
- A C++17 compiler

## Dependencies

Fetch pinned dependencies:

```sh
./gdext/setup_deps.sh
```

This clones Box3D and godot-cpp into `gdext/extern/`. They are vendored working
copies, not git submodules.

The generated binding sources are committed. Verify they match the pinned
Box3D headers with:

```sh
python3 tools/generate_bindings.py --check
```

Run `python3 tools/generate_bindings.py` only when intentionally changing the
Box3D pin or generator.

## macOS and Linux

```sh
cd gdext
cmake -S . -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DGODOTCPP_TARGET=template_debug
cmake --build build-debug
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DGODOTCPP_TARGET=template_release
cmake --build build-release
```

## Windows

Run from a Visual Studio 2022 x64 developer shell:

```bat
cd gdext
cmake -S . -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DGODOTCPP_TARGET=template_debug
cmake --build build-debug
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DGODOTCPP_TARGET=template_release
cmake --build build-release
```

The build uses the static MSVC runtime and `/fp:precise`.

## Output

Binaries are written to `addons/box3d_rollback/bin/`, where
`addons/box3d_rollback/box3d_rollback.gdextension` expects them.

When this repository is mounted as a source dependency, set
`BOX3D_ROLLBACK_OUTPUT_DIR` to the consuming Godot project's
`addons/box3d_rollback/bin` directory.
