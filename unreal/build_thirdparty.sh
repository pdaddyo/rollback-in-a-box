#!/bin/sh
# Build box3d + the engine-neutral rollback core as static libraries and stage
# them where the Unreal module's Build.cs expects them.
#
# Run from anywhere. Requires cmake + a C/C++ toolchain (and, on Windows, an
# MSVC dev environment). Fetches the pinned box3d if it is missing.
set -e
cd "$(dirname "$0")/.."   # repo root

if [ ! -d gdext/extern/box3d ]; then
    echo "== fetching pinned dependencies =="
    ./gdext/setup_deps.sh
fi

BUILD_DIR=unreal/thirdparty/build
echo "== configuring =="
cmake -S unreal/thirdparty -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
echo "== building =="
cmake --build "$BUILD_DIR" --config Release

# UBT's Target.Platform.ToString(): Win64 / Mac / Linux.
case "$(uname -s)" in
    Linux*)  PLAT=Linux ;;
    Darwin*) PLAT=Mac ;;
    MINGW*|MSYS*|CYGWIN*) PLAT=Win64 ;;
    *)       PLAT=Win64 ;;
esac

DEST="unreal/Box3DRollback/Source/ThirdParty/Box3DRollbackLibrary/lib/$PLAT"
mkdir -p "$DEST"

copied=0
for name in libbox3d_rollback_neutral.a libbox3d.a box3d_rollback_neutral.lib box3d.lib; do
    found=$(find "$BUILD_DIR" -name "$name" | head -n 1 || true)
    if [ -n "$found" ]; then
        cp "$found" "$DEST/"
        copied=$((copied + 1))
    fi
done

if [ "$copied" -eq 0 ]; then
    echo "ERROR: no static libraries were produced under $BUILD_DIR" >&2
    exit 1
fi

echo "== staged $copied static libraries in $DEST =="
