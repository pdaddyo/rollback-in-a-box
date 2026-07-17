cmake_minimum_required(VERSION 3.22)
project(box3d_rollback LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")

if(NOT MSVC)
    add_compile_options(-ffp-contract=off)
else()
    add_compile_options(/fp:precise)
endif()

add_subdirectory(extern/box3d)
add_subdirectory(extern/godot-cpp)

# Engine-neutral rollback core (rb::RollbackWorld / rb::RollbackSession + the
# snapshot shim). Depends on box3d only, and is the exact same code the Unreal
# plugin compiles. The Godot classes below are thin adapters over it.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core ${CMAKE_CURRENT_BINARY_DIR}/core)

option(BOX3D_ROLLBACK_BUILD_EXTENSION "Build the standalone GDExtension library" ON)

add_library(box3d_rollback_core OBJECT
    src/box3d_rollback_classes.cpp
    src/box3d_raw.cpp
    src/box3d_rollback_world.cpp
    src/box3d_rollback_session.cpp
    src/box3d_replay.cpp
)

target_include_directories(box3d_rollback_core PUBLIC
    src
    extern/box3d/src
)

target_link_libraries(box3d_rollback_core PUBLIC godot-cpp box3d box3d_rollback_neutral)

set_target_properties(box3d_rollback_core PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

if(NOT BOX3D_ROLLBACK_BUILD_EXTENSION)
    return()
endif()

add_library(box3d_rollback SHARED
    src/register_types.cpp
    $<TARGET_OBJECTS:box3d_rollback_core>
)

target_include_directories(box3d_rollback PRIVATE src)
target_link_libraries(box3d_rollback PRIVATE godot-cpp box3d box3d_rollback_neutral)

get_target_property(GODOTCPP_SUFFIX godot-cpp GODOTCPP_SUFFIX)
set(BOX3D_ROLLBACK_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../addons/box3d_rollback/bin"
    CACHE PATH "Directory where the Godot extension binary is written")

set_target_properties(box3d_rollback PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    BUILD_RPATH_USE_ORIGIN ON
    LIBRARY_OUTPUT_DIRECTORY "$<1:${BOX3D_ROLLBACK_OUTPUT_DIR}>"
    RUNTIME_OUTPUT_DIRECTORY "$<1:${BOX3D_ROLLBACK_OUTPUT_DIR}>"
    PREFIX "lib"
    OUTPUT_NAME "box3d_rollback${GODOTCPP_SUFFIX}"
)
