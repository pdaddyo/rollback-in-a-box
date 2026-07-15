extends SceneTree

var api := Box3D.new()
var raw: Box3DRaw
var constants: Dictionary
var world: int
var resources: Array[int] = []

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

func expect_shape(shape: int, expected_type: String) -> void:
	if not api.b3Shape_IsValid(shape):
		return fail("%s shape is invalid" % expected_type)
	if api.b3Shape_GetType(shape) != constants[expected_type]:
		return fail("%s shape type mismatch" % expected_type)

func create_all_shapes() -> void:
	var sphere_body := create_body(constants["b3_dynamicBody"], Vector3(-5, 3, 0))
	var sphere: int = api.b3CreateSphereShape(
		sphere_body,
		shape_definition(),
		make_buffer(&"b3Sphere", {"center": Vector3.ZERO, "radius": 0.5})
	)
	expect_shape(sphere, "b3_sphereShape")

	var capsule_body := create_body(constants["b3_dynamicBody"], Vector3(-3, 3, 0))
	var capsule: int = api.b3CreateCapsuleShape(
		capsule_body,
		shape_definition(),
		make_buffer(&"b3Capsule", {"center1": Vector3(0, -0.5, 0), "center2": Vector3(0, 0.5, 0), "radius": 0.25})
	)
	expect_shape(capsule, "b3_capsuleShape")

	var hull_body := create_body(constants["b3_dynamicBody"], Vector3(-1, 3, 0))
	var box_hull: Dictionary = api.b3MakeBoxHull(0.5, 0.5, 0.5)
	var hull: int = api.b3CreateHullShape(hull_body, shape_definition(), make_buffer(&"b3HullData", box_hull["base"]))
	expect_shape(hull, "b3_hullShape")

	var mesh_body := create_body(constants["b3_staticBody"], Vector3(1, 0, 0))
	var mesh_data: int = api.b3CreateBoxMesh(Vector3.ZERO, Vector3(0.75, 0.25, 0.75), true)
	resources.append(mesh_data)
	var mesh: int = api.b3CreateMeshShape(mesh_body, shape_definition(0.0), mesh_data, Vector3.ONE)
	expect_shape(mesh, "b3_meshShape")

	var height_body := create_body(constants["b3_staticBody"], Vector3(4, 0, 0))
	var height_data: int = api.b3CreateGrid(4, 4, Vector3(0.5, 0.25, 0.5), false)
	resources.append(height_data)
	var height: int = api.b3CreateHeightFieldShape(height_body, shape_definition(0.0), height_data)
	expect_shape(height, "b3_heightShape")

	var child: Box3DBuffer = make_buffer(&"b3CompoundSphereDef", {
		"sphere": {"center": Vector3.ZERO, "radius": 0.6},
		"material": api.b3DefaultSurfaceMaterial(),
	})
	var compound_definition: Box3DBuffer = make_buffer(&"b3CompoundDef", {
		"spheres": child.get_address(),
		"sphereCount": 1,
	})
	var compound_data: int = api.b3CreateCompound(compound_definition)
	resources.append(compound_data)
	var compound_body := create_body(constants["b3_staticBody"], Vector3(7, 1, 0))
	var compound: int = api.b3CreateBakedCompoundShape(compound_body, shape_definition(0.0), compound_data)
	expect_shape(compound, "b3_compoundShape")

func create_joint(default_function: StringName, definition_type: StringName, create_function: StringName,
		expected_type: String, body_a: int, body_b: int) -> int:
	var definition: Dictionary = raw.call_box3d(default_function, [])
	var base: Dictionary = definition["base"]
	base["bodyIdA"] = body_a
	base["bodyIdB"] = body_b
	definition["base"] = base
	var joint: int = raw.call_box3d(create_function, [world, make_buffer(definition_type, definition)])
	if not api.b3Joint_IsValid(joint):
		fail("%s creation failed" % expected_type)
		return 0
	if api.b3Joint_GetType(joint) != constants[expected_type]:
		fail("%s type mismatch" % expected_type)
		return 0
	return joint

func create_all_joints() -> void:
	var specs := [
		[&"b3DefaultParallelJointDef", &"b3ParallelJointDef", &"b3CreateParallelJoint", "b3_parallelJoint"],
		[&"b3DefaultDistanceJointDef", &"b3DistanceJointDef", &"b3CreateDistanceJoint", "b3_distanceJoint"],
		[&"b3DefaultMotorJointDef", &"b3MotorJointDef", &"b3CreateMotorJoint", "b3_motorJoint"],
		[&"b3DefaultFilterJointDef", &"b3FilterJointDef", &"b3CreateFilterJoint", "b3_filterJoint"],
		[&"b3DefaultPrismaticJointDef", &"b3PrismaticJointDef", &"b3CreatePrismaticJoint", "b3_prismaticJoint"],
		[&"b3DefaultRevoluteJointDef", &"b3RevoluteJointDef", &"b3CreateRevoluteJoint", "b3_revoluteJoint"],
		[&"b3DefaultSphericalJointDef", &"b3SphericalJointDef", &"b3CreateSphericalJoint", "b3_sphericalJoint"],
		[&"b3DefaultWeldJointDef", &"b3WeldJointDef", &"b3CreateWeldJoint", "b3_weldJoint"],
		[&"b3DefaultWheelJointDef", &"b3WheelJointDef", &"b3CreateWheelJoint", "b3_wheelJoint"],
	]
	var joints: Array[int] = []
	for i in range(specs.size()):
		var body_a := create_body(constants["b3_dynamicBody"], Vector3(20 + i * 3, 2, 0))
		var body_b := create_body(constants["b3_dynamicBody"], Vector3(21 + i * 3, 2, 0))
		add_body_sphere(body_a)
		add_body_sphere(body_b)
		joints.append(create_joint(specs[i][0], specs[i][1], specs[i][2], specs[i][3], body_a, body_b))

	api.b3ParallelJoint_SetSpringHertz(joints[0], 3.0)
	api.b3DistanceJoint_SetLength(joints[1], 1.25)
	api.b3MotorJoint_SetLinearVelocity(joints[2], Vector3(1, 0, 0))
	api.b3Joint_SetCollideConnected(joints[3], true)
	api.b3PrismaticJoint_SetMotorSpeed(joints[4], 2.0)
	api.b3RevoluteJoint_SetMotorSpeed(joints[5], 1.5)
	api.b3SphericalJoint_SetConeLimit(joints[6], 0.75)
	api.b3WeldJoint_SetLinearHertz(joints[7], 4.0)
	api.b3WheelJoint_SetSuspensionHertz(joints[8], 5.0)
	if not is_equal_approx(api.b3ParallelJoint_GetSpringHertz(joints[0]), 3.0): return fail("parallel accessor failed")
	if not is_equal_approx(api.b3DistanceJoint_GetLength(joints[1]), 1.25): return fail("distance accessor failed")
	if api.b3MotorJoint_GetLinearVelocity(joints[2]) != Vector3(1, 0, 0): return fail("motor accessor failed")
	if not api.b3Joint_GetCollideConnected(joints[3]): return fail("filter/generic accessor failed")
	if not is_equal_approx(api.b3PrismaticJoint_GetMotorSpeed(joints[4]), 2.0): return fail("prismatic accessor failed")
	if not is_equal_approx(api.b3RevoluteJoint_GetMotorSpeed(joints[5]), 1.5): return fail("revolute accessor failed")
	if not is_equal_approx(api.b3SphericalJoint_GetConeLimit(joints[6]), 0.75): return fail("spherical accessor failed")
	if not is_equal_approx(api.b3WeldJoint_GetLinearHertz(joints[7]), 4.0): return fail("weld accessor failed")
	if not is_equal_approx(api.b3WheelJoint_GetSuspensionHertz(joints[8]), 5.0): return fail("wheel accessor failed")

func _initialize() -> void:
	raw = api.raw
	constants = raw.get_constants()
	if raw.get_api_function_count() < 695:
		return fail("public API catalog is incomplete")
	for required in ["b3CreateSphereShape", "b3CreateBakedCompoundShape", "b3CreateWheelJoint", "b3ShapeDistance", "b3World_GetContactEvents"]:
		if not raw.get_functions().has(required):
			return fail("missing API function %s" % required)

	var world_definition: Dictionary = api.b3DefaultWorldDef()
	world_definition["gravity"] = Vector3(0, -9.8, 0)
	world = api.b3CreateWorld(make_buffer(&"b3WorldDef", world_definition))
	if not api.b3World_IsValid(world):
		return fail("world creation failed")

	create_all_shapes()
	create_all_joints()
	for _i in range(8):
		api.b3World_Step(world, 1.0 / 60.0, 4)
	var bounds: Dictionary = api.b3World_GetBounds(world)
	if not bounds.has("lowerBound") or not bounds.has("upperBound"):
		return fail("world bounds codec failed")
	var ray: Dictionary = api.b3World_CastRayClosest(world, Vector3(0, 20, 0), Vector3(0, -40, 0), api.b3DefaultQueryFilter())
	if not ray.has("hit"):
		return fail("world query codec failed")

	api.b3DestroyWorld(world)
	api.b3DestroyMesh(resources[0])
	api.b3DestroyHeightField(resources[1])
	api.b3DestroyCompound(resources[2])
	print("raw api: OK functions=%d shapes=6 joints=9" % raw.get_api_function_count())
	quit(0)
