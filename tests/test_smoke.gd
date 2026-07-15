extends SceneTree

func fail(msg: String) -> void:
	push_error("FAIL: " + msg)
	quit(1)

func _initialize() -> void:
	var world := Box3DRollbackWorld.new()
	root.add_child(world)
	world.set_input_count(2)
	world.create_world()

	world.add_static_box(Vector3(0, -1, 0), Vector3(20, 1, 20))
	var box := world.add_dynamic_box(Vector3(0, 4, 0), Vector3(0.5, 0.5, 0.5))
	world.add_dynamic_sphere(Vector3(2, 5, 0), 0.4)
	world.add_dynamic_capsule(Vector3(-2, 5, 0), Vector3(0, -0.5, 0), Vector3(0, 0.5, 0), 0.25)
	world.set_body_linear_velocity(box, Vector3(1, 0, 0))

	world.init_snapshots(8)
	if not world.save_state(0):
		return fail("save_state(0) failed")
	var h0: int = world.state_hash()

	var idle := PackedInt64Array([0, 0])
	for i in range(30):
		world.step_frame(idle)
	if world.get_frame() != 30:
		return fail("frame counter wrong")
	var h30: int = world.state_hash()
	if h30 == h0:
		return fail("hash did not change after stepping")

	if not world.load_state(0):
		return fail("load_state(0) failed")
	if world.get_frame() != 0:
		return fail("frame did not restore")
	if world.state_hash() != h0:
		return fail("hash did not restore")

	for i in range(30):
		world.step_frame(idle)
	if world.state_hash() != h30:
		return fail("resim hash mismatch")
	if world.get_transforms().size() != world.get_body_count() * 7:
		return fail("transform stream size mismatch")
	if world.get_body_meta().size() != world.get_body_count() * 4:
		return fail("body metadata stream size mismatch")

	print("smoke: OK")
	quit(0)
