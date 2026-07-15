# rollback-in-a-box

Deterministic network rollback physics for Godot, powered by [Box3D](https://github.com/erincatto/box3d) by Erin Catto.

`rollback-in-a-box` is a Godot 4 GDExtension with three deliberately separate
layers:

- The complete public Box3D C API, generated into a Godot-facing `Box3D` class.
- Deterministic full-world snapshots, hashes, recording, and replay.
- Transport-agnostic rollback networking with prediction, input resend, acking,
  confirmed-frame hashes, desync detection, and frame-advantage throttling.

Your game owns game rules, rendering, matchmaking, and byte transport. The
extension owns physics state, rollback mechanics, and the rollback protocol.

## API coverage

The pinned Box3D revision currently exposes 700 public functions: 577 exported
`B3_API` functions and 123 public inline helpers. All 700 are represented by
the generated `Box3D` class with their exact C names. The generated
[API reference](docs/api-reference.md) and [machine-readable manifest](api/box3d-api.json)
are checked in CI, so an upstream API change cannot silently leave the binding
incomplete.

The behavioral suite creates every runtime shape family and every joint family
through this raw API. `Box3DRollbackWorld` remains a smaller convenience and
snapshot layer; it is not the boundary of Box3D coverage.

## Quick start

```sh
./gdext/setup_deps.sh
cmake -S gdext -B gdext/build-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DGODOTCPP_TARGET=template_debug
cmake --build gdext/build-debug
./tests/run_all.sh
```

Create a world through the complete raw API:

```gdscript
var box3d := Box3D.new()
var world_def: Dictionary = box3d.b3DefaultWorldDef()
world_def.gravity = Vector3(0, -9.8, 0)
var world_buffer := box3d.raw.make_buffer(&"b3WorldDef", world_def)
var world_id: int = box3d.b3CreateWorld(world_buffer)

var body_def: Dictionary = box3d.b3DefaultBodyDef()
body_def.type = box3d.raw.get_constants().b3_dynamicBody
body_def.position = Vector3(0, 4, 0)
var body_id: int = box3d.b3CreateBody(
  world_id,
  box3d.raw.make_buffer(&"b3BodyDef", body_def)
)

var shape_def := box3d.raw.make_buffer(&"b3ShapeDef", box3d.b3DefaultShapeDef())
var sphere := box3d.raw.make_buffer(&"b3Sphere", {
  "center": Vector3.ZERO,
  "radius": 0.5,
})
box3d.b3CreateSphereShape(body_id, shape_def, sphere)
box3d.b3World_Step(world_id, 1.0 / 60.0, 4)
```

Or use the deterministic convenience world and network session:

```gdscript
var world := Box3DRollbackWorld.new()
add_child(world)
world.set_input_count(2)
world.create_world()
world.add_static_box(Vector3(0, -1, 0), Vector3(20, 1, 20))
world.add_dynamic_box(Vector3(0, 4, 0), Vector3(0.5, 0.5, 0.5))

var session := Box3DRollbackSession.new()
session.set_simulation(world)
session.configure(0, 2, 2, 8)
session.start()
```

Send `session.get_packet()` through UDP, ENet, WebRTC, Steam networking, or a
relay. Pass received bytes unchanged to `session.ingest_packet(packet)`.

## Documentation

- [Raw Box3D API](docs/raw-box3d-api.md)
- [API boundary](docs/api-boundary.md)
- [Generated API reference](docs/api-reference.md)
- [Build from source](docs/build-from-source.md)
- [Determinism contract](docs/determinism-contract.md)
- [Network rollback](docs/network-rollback.md)
- [Transport adapters](docs/transport-adapters.md)
- [Snapshots and hashing](docs/snapshots-and-hashing.md)
- [Testing](docs/testing.md)

## License

MIT. Box3D and godot-cpp are fetched at pinned upstream revisions and retain
their own licenses.
