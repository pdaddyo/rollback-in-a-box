cmake_minimum_required(VERSION 3.22)
project(box3d_rollback_unreal_thirdparty LANGUAGES C CXX)

# Builds the two static libraries the Unreal module links against:
#   - box3d                    (the vendored physics engine)
#   - box3d_rollback_neutral   (the engine-neutral rollback core, repo /core)
#
# This is the SAME core the Godot GDExtension compiles — there is no second copy
# of the rollback logic. Invoke via unreal/build_thirdparty.sh (or the cmake
# commands in unreal/Box3DRollback/README.md).

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# Unreal on Windows links the DYNAMIC CRT (/MD, /MDd). Third-party static libs
# linked into a UE module must match, or std::vector buffers allocated in the
# core and freed in the module cross CRT heaps and crash. (The Godot build uses
# /MT to make a self-contained GDExtension DLL — a different, separate target.)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>" CACHE STRING "")

# Determinism contract — identical to gdext/CMakeLists.txt and core/CMakeLists.txt.
if(NOT MSVC)
    add_compile_options(-ffp-contract=off)
else()
    add_compile_options(/fp:precise)
endif()

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# box3d must be fetched first: run ./gdext/setup_deps.sh from the repo root.
add_subdirectory(${REPO_ROOT}/gdext/extern/box3d ${CMAKE_CURRENT_BINARY_DIR}/box3d)
add_subdirectory(${REPO_ROOT}/core ${CMAKE_CURRENT_BINARY_DIR}/core)
