extends SceneTree

# Edge cases: garbage/stale ids, degenerate step inputs, distance joint
# clamping, legal wrong-family shape mutation, degenerate AABB validation,
# and (in a child process) the joint family-mismatch assertion.

var api := Box3D.new()
var raw: Box3DRaw
var constants: Dictionary
var world: int

func fail(message: String) -> void:
	push_error("FAIL: " + message)
	quit(1)

func make_buffer(type_name: StringName, value: Variant):
	var buffer = raw.make_buffer(type_name, value)
	if buffer == null:
		fail("could not encode %s" % type_name)
	return buffer

func create_body(body_type: int, position: Vector3) -> int:
	var definition: Dictionary = api.b3DefaultBodyDef()
	definition["type"] = body_type
	definition["position"] = position
	var body: int = api.b3CreateBody(world, make_buffer(&"b3BodyDef", definition))
	if not api.b3Body_IsValid(body):
		fail("body creation failed")
	return body

func shape_definition(density := 1.0):
	var definition: Dictionary = api.b3DefaultShapeDef()
	definition["density"] = density
	return make_buffer(&"b3ShapeDef", definition)

func add_body_sphere(body: int, radius := 0.25) -> int:
	var sphere: Box3DBuffer = make_buffer(&"b3Sphere", {"center": Vector3.ZERO, "radius": radius})
	return api.b3CreateSphereShape(body, shape_definition(), sphere)

func check_garbage_ids() -> void:
	# Bounds/generation checks reject these without asserting. Note id 0 is
	# always invalid (encoded index is 1-based), even while a world is live.
	var garbage := [0, -1, 123456]
	for id in garbage:
		if api.b3World_IsValid(id):
			return fail("b3World_IsValid(%d) should be false" % id)
	for id in [0, -1, 1 << 40]:
		if api.b3Body_IsValid(id):
			return fail("b3Body_IsValid(%d) should be false" % id)
		if api.b3Shape_IsValid(id):
			return fail("b3Shape_IsValid(%d) should be false" % id)
		if api.b3Joint_IsValid(id):
			return fail("b3Joint_IsValid(%d) should be false" % id)

func check_stale_world_id() -> void:
	var stale: int = api.b3CreateWorld(make_buffer(&"b3WorldDef", api.b3DefaultWorldDef()))
	if not api.b3World_IsValid(stale):
		return fail("second world creation failed")
	api.b3DestroyWorld(stale)
	if api.b3World_IsValid(stale):
		return fail("destroyed world id should be invalid")
	# Re-create a world; the old id must not alias the recycled slot
	# (generation is bumped on destroy).
	var replacement: int = api.b3CreateWorld(make_buffer(&"b3WorldDef", api.b3DefaultWorldDef()))
	if not api.b3World_IsValid(replacement):
		return fail("replacement world creation failed")
	if api.b3World_IsValid(stale):
		return fail("stale world id aliases a new world")
	if stale == replacement:
		return fail("stale world id was recycled verbatim")
	api.b3DestroyWorld(replacement)

func check_destroyed_body_and_shape() -> void:
	var body := create_body(constants["b3_dynamicBody"], Vector3(0, 10, 0))
	var shape := add_body_sphere(body)
	if not api.b3Shape_IsValid(shape):
		return fail("sphere shape creation failed")
	api.b3DestroyBody(body)
	if api.b3Body_IsValid(body):
		return fail("destroyed body id should be invalid")
	if api.b3Shape_IsValid(shape):
		return fail("shape of destroyed body should be invalid")

func check_zero_dt_step() -> void:
	var body := create_body(constants["b3_dynamicBody"], Vector3(0, 20, 0))
	add_body_sphere(body)
	api.b3Body_SetLinearVelocity(body, Vector3(3, 0, 0))
	var before: Vector3 = api.b3Body_GetPosition(body)
	# subStepCount is clamped to >= 1 and timeStep <= 0 yields h = 0.
	api.b3World_Step(world, 0.0, 0)
	var after: Vector3 = api.b3Body_GetPosition(body)
	if before != after:
		return fail("zero-dt step moved the body: %s -> %s" % [before, after])
	api.b3DestroyBody(body)

func check_distance_joint_clamps() -> void:
	var body_a := create_body(constants["b3_dynamicBody"], Vector3(30, 2, 0))
	var body_b := create_body(constants["b3_dynamicBody"], Vector3(31, 2, 0))
	add_body_sphere(body_a)
	add_body_sphere(body_b)
	var definition: Dictionary = api.b3DefaultDistanceJointDef()
	var base: Dictionary = definition["base"]
	base["bodyIdA"] = body_a
	base["bodyIdB"] = body_b
	definition["base"] = base
	var joint: int = api.b3CreateDistanceJoint(world, make_buffer(&"b3DistanceJointDef", definition))
	if not api.b3Joint_IsValid(joint):
		return fail("distance joint creation failed")

	# Negative length clamps up to B3_LINEAR_SLOP (0.005).
	api.b3DistanceJoint_SetLength(joint, -5.0)
	var length: float = api.b3DistanceJoint_GetLength(joint)
	if not is_equal_approx(length, 0.005):
		return fail("negative length should clamp to 0.005, got %f" % length)

	# Inverted range (min > max) is normalized.
	api.b3DistanceJoint_SetLengthRange(joint, 5.0, 1.0)
	var min_length: float = api.b3DistanceJoint_GetMinLength(joint)
	var max_length: float = api.b3DistanceJoint_GetMaxLength(joint)
	if not is_equal_approx(min_length, 1.0):
		return fail("min length should normalize to 1.0, got %f" % min_length)
	if not is_equal_approx(max_length, 5.0):
		return fail("max length should normalize to 5.0, got %f" % max_length)

func check_set_sphere_on_hull() -> void:
	# Mutating a hull shape into a sphere via b3Shape_SetSphere is a
	# documented legal operation (unlike wrong-family getters, which assert).
	var body := create_body(constants["b3_dynamicBody"], Vector3(40, 2, 0))
	var box_hull: Dictionary = api.b3MakeBoxHull(0.5, 0.5, 0.5)
	var hull: int = api.b3CreateHullShape(body, shape_definition(), make_buffer(&"b3HullData", box_hull["base"]))
	if not api.b3Shape_IsValid(hull):
		return fail("hull shape creation failed")
	if api.b3Shape_GetType(hull) != constants["b3_hullShape"]:
		return fail("hull shape type mismatch before mutation")

	api.b3Shape_SetSphere(hull, make_buffer(&"b3Sphere", {"center": Vector3.ZERO, "radius": 0.5}))
	if api.b3Shape_GetType(hull) != constants["b3_sphereShape"]:
		return fail("shape type should be sphere after b3Shape_SetSphere")
	var sphere: Dictionary = api.b3Shape_GetSphere(hull)
	if not is_equal_approx(float(sphere["radius"]), 0.5):
		return fail("mutated sphere radius should be 0.5, got %s" % str(sphere["radius"]))

func check_degenerate_aabbs() -> void:
	var inverted := {"lowerBound": Vector3(1, 1, 1), "upperBound": Vector3(0, 0, 0)}
	if api.b3IsValidAABB(inverted):
		return fail("inverted AABB should be invalid")
	var with_nan := {"lowerBound": Vector3(NAN, 0, 0), "upperBound": Vector3(1, 1, 1)}
	if api.b3IsValidAABB(with_nan):
		return fail("NaN AABB should be invalid")
	var with_inf := {"lowerBound": Vector3.ZERO, "upperBound": Vector3(INF, 1, 1)}
	if api.b3IsValidAABB(with_inf):
		return fail("INF AABB should be invalid")
	var point := {"lowerBound": Vector3(2, 3, 4), "upperBound": Vector3(2, 3, 4)}
	if not api.b3IsValidAABB(point):
		return fail("point AABB (lower == upper) should be valid")

const CHILD_SCRIPT := """
extends SceneTree

func _initialize() -> void:
	var raw := Box3DRaw.new()
	var world_def: Dictionary = raw.call_box3d(&\"b3DefaultWorldDef\", [])
	var world: int = raw.call_box3d(&\"b3CreateWorld\", [raw.make_buffer(&\"b3WorldDef\", world_def)])
	var constants: Dictionary = raw.get_constants()
	var ids: Array[int] = []
	for x in [0.0, 1.0]:
		var body_def: Dictionary = raw.call_box3d(&\"b3DefaultBodyDef\", [])
		body_def[\"type\"] = constants[\"b3_dynamicBody\"]
		body_def[\"position\"] = Vector3(x, 2, 0)
		var body: int = raw.call_box3d(&\"b3CreateBody\", [world, raw.make_buffer(&\"b3BodyDef\", body_def)])
		var shape_def: Dictionary = raw.call_box3d(&\"b3DefaultShapeDef\", [])
		raw.call_box3d(&\"b3CreateSphereShape\", [body, raw.make_buffer(&\"b3ShapeDef\", shape_def),
				raw.make_buffer(&\"b3Sphere\", {\"center\": Vector3.ZERO, \"radius\": 0.25})])
		ids.append(body)
	var joint_def: Dictionary = raw.call_box3d(&\"b3DefaultRevoluteJointDef\", [])
	var base: Dictionary = joint_def[\"base\"]
	base[\"bodyIdA\"] = ids[0]
	base[\"bodyIdB\"] = ids[1]
	joint_def[\"base\"] = base
	var joint: int = raw.call_box3d(&\"b3CreateRevoluteJoint\", [world, raw.make_buffer(&\"b3RevoluteJointDef\", joint_def)])
	# Family mismatch: distance-joint getter on a revolute joint must assert.
	raw.call_box3d(&\"b3DistanceJoint_GetLength\", [joint])
	print(\"CHILD_SURVIVED\")
	quit(0)
"""

func check_joint_family_mismatch_in_child() -> void:
	# The mismatch triggers B3_ASSERT -> __builtin_trap, which would kill this
	# process, so it runs in a second headless Godot.
	var script_path: String = OS.get_cache_dir().path_join("box3d_edge_joint_mismatch_child.gd")
	var file := FileAccess.open(script_path, FileAccess.WRITE)
	if file == null:
		return fail("could not write child script to %s" % script_path)
	file.store_string(CHILD_SCRIPT)
	file.close()

	var project_root: String = ProjectSettings.globalize_path("res://")
	var output: Array = []
	var exit_code: int = OS.execute(OS.get_executable_path(),
			["--headless", "--path", project_root, "-s", script_path], output, true)
	DirAccess.remove_absolute(script_path)

	var combined := ""
	for chunk in output:
		combined += str(chunk)
	if combined.contains("CHILD_SURVIVED"):
		return fail("joint family mismatch did not assert in child process")
	if not combined.contains("BOX3D ASSERTION"):
		return fail("child output missing BOX3D ASSERTION (exit=%d): %s" % [exit_code, combined])
	if exit_code == 0:
		return fail("child process should exit nonzero on assertion")

func _initialize() -> void:
	raw = api.raw
	constants = raw.get_constants()

	var world_definition: Dictionary = api.b3DefaultWorldDef()
	world_definition["gravity"] = Vector3(0, -9.8, 0)
	world = api.b3CreateWorld(make_buffer(&"b3WorldDef", world_definition))
	if not api.b3World_IsValid(world):
		return fail("world creation failed")

	check_garbage_ids()
	check_stale_world_id()
	check_destroyed_body_and_shape()
	check_zero_dt_step()
	check_distance_joint_clamps()
	check_set_sphere_on_hull()
	check_degenerate_aabbs()
	check_joint_family_mismatch_in_child()

	api.b3DestroyWorld(world)
	print("edge ids/shapes: OK")
	quit(0)
