cmake_minimum_required(VERSION 3.22)

# Engine-neutral Box3D rollback core.
#
# Depends only on the `box3d` target (which the including project must define,
# e.g. via add_subdirectory(extern/box3d)). No godot-cpp, no Unreal, no Godot
# types. Both the Godot GDExtension and the Unreal plugin build against this.
#
# The Unreal plugin does NOT use this CMake file — UBT compiles the same sources
# directly (see unreal/Box3DRollback/Source/.../Box3DRollback.Build.cs). This
# file is the CMake entry point used by the Godot build and standalone C++ hosts.

if(NOT TARGET box3d)
    message(FATAL_ERROR
        "box3d_rollback_neutral requires the `box3d` CMake target. "
        "add_subdirectory(<box3d>) before add_subdirectory(core).")
endif()

add_library(box3d_rollback_neutral STATIC
    src/rb_session.cpp
    src/rb_world.cpp
    src/rollback_shim.c
)

# The snapshot shim reaches box3d's private headers (core.h, world_snapshot.h,
# ...), the same internal machinery the recording system uses. Derive the path
# from the box3d target so this file never hardcodes where box3d was cloned.
# The `box3d` target is defined in box3d/src/CMakeLists.txt, so its SOURCE_DIR is
# already box3d/src; include both forms so this is robust to either layout.
get_target_property(_b3_source_dir box3d SOURCE_DIR)

target_include_directories(box3d_rollback_neutral PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/box3d_rollback
)
target_include_directories(box3d_rollback_neutral PRIVATE
    ${_b3_source_dir}
    ${_b3_source_dir}/src
)

target_link_libraries(box3d_rollback_neutral PUBLIC box3d)

set_target_properties(box3d_rollback_neutral PROPERTIES
    C_STANDARD 17
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

# Determinism contract: never let the compiler contract or reassociate FP in
# simulation code. Kept on the target itself so a standalone/Unreal CMake build
# is correct even without the parent's global flags. See docs/determinism-contract.md.
if(NOT MSVC)
    target_compile_options(box3d_rollback_neutral PRIVATE -ffp-contract=off)
else()
    target_compile_options(box3d_rollback_neutral PRIVATE /fp:precise)
endif()
