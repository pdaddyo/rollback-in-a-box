# Raw Box3D API

`Box3D` exposes every function found in the pinned public Box3D headers with
its exact C name. `Box3DRaw` supplies reflection, struct encoding, constants,
and the native dispatcher beneath it.

## Values and ids

The generated binding maps common value types directly:

| Box3D | Godot |
| --- | --- |
| `b3WorldId`, `b3BodyId`, `b3ShapeId`, `b3JointId` | `int` |
| `b3Vec3`, `b3Pos` | `Vector3` |
| `b3Quat` | `Quaternion` |
| `b3Transform`, `b3WorldTransform` | `Transform3D` |
| enum | `int`, available from `Box3DRaw.get_constants()` |
| other value struct | `Dictionary` or exact `PackedByteArray` |

`raw.get_struct_info(&"b3BodyDef")` returns the native size and field list.
`raw.get_function_info(&"b3CreateBody")` returns the exact C signature.

## Pointer parameters

C functions that take a pointer require one of:

- a `Box3DBuffer` created by `make_buffer()` or `make_byte_buffer()`
- an integer native address returned by a Box3D creation function
- `null` where the C API permits null

```gdscript
var body_def: Dictionary = box3d.b3DefaultBodyDef()
body_def.type = constants.b3_dynamicBody
var body_buffer := box3d.raw.make_buffer(&"b3BodyDef", body_def)
var body_id := box3d.b3CreateBody(world_id, body_buffer)
```

For output arrays, allocate an owned buffer, pass it to the function, then read
the bytes back:

```gdscript
var count: int = box3d.b3Body_GetShapeCount(body_id)
var id_size: int = box3d.raw.get_struct_info(&"b3ShapeId").size
var output := box3d.raw.make_byte_buffer(count * id_size)
var written: int = box3d.b3Body_GetShapes(body_id, output, count)
var bytes: PackedByteArray = output.data
```

Addresses are process-local. Never serialize them, hash them as gameplay
state, or include them in network packets.

## Resource ownership

Functions returning geometry pointers transfer an address, not a Godot object.
Keep the address while shapes reference that geometry and call the matching
destroy function after the world no longer uses it:

```gdscript
var mesh: int = box3d.b3CreateBoxMesh(Vector3.ZERO, Vector3.ONE, true)
box3d.b3CreateMeshShape(body_id, shape_def, mesh, Vector3.ONE)
# Destroy the world or its mesh shapes first.
box3d.b3DestroyMesh(mesh)
```

The same rule applies to hull, mesh, height-field, compound, recording, replay,
and dynamic-tree resources.

## Callbacks

The raw ABI accepts native callback addresses exactly as Box3D does. GDScript
`Callable` values are intentionally not installed as simulation callbacks:
calling arbitrary script during physics would make deterministic scheduling and
lifetime guarantees ambiguous. Use array-returning convenience adapters or a
small C++ adapter when a project needs custom filter, pre-solve, allocator,
task, or debug-draw callbacks.

## Generated coverage

Run:

```sh
python3 tools/generate_bindings.py --check
```

The command reparses the pinned public headers and fails if the checked-in C++
dispatcher, GDScript facade, API manifest, or reference documentation is stale.
To intentionally update the pin, run the generator without `--check`, inspect
the manifest diff, build all platforms, and run the complete suite.
