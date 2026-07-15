#!/bin/sh
# Clone the two vendored dependencies at the pinned, known-good revisions.
# Run from anywhere; clones into gdext/extern/.
set -e
cd "$(dirname "$0")"

BOX3D_SHA=540ea387b0c02bf714fbfdcc8fb88c039c35fe6f
GODOT_CPP_TAG=godot-4.5-stable

if [ ! -d extern/box3d ]; then
    git clone https://github.com/erincatto/box3d.git extern/box3d
    git -C extern/box3d checkout --detach "$BOX3D_SHA"
else
    echo "extern/box3d already present ($(git -C extern/box3d rev-parse --short HEAD))"
fi

if [ ! -d extern/godot-cpp ]; then
    git clone --depth 1 --branch "$GODOT_CPP_TAG" https://github.com/godotengine/godot-cpp.git extern/godot-cpp
else
    echo "extern/godot-cpp already present ($(git -C extern/godot-cpp describe --tags 2>/dev/null || echo '?'))"
fi

echo "deps ready"
