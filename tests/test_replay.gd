extends SceneTree

func fail(message: String) -> void:
	push_error("FAIL: " + message)
	quit(1)

func _initialize() -> void:
	var path := ProjectSettings.globalize_path("user://rollback_in_a_box_test.b3r")
	var world := Box3DRollbackWorld.new()
	root.add_child(world)
	world.set_input_count(1)
	world.create_world()
	world.add_static_box(Vector3(0, -1, 0), Vector3(10, 1, 10))
	var body := world.add_dynamic_sphere(Vector3(0, 4, 0), 0.5)
	world.set_body_linear_velocity(body, Vector3(1, 0, 0))
	if not world.start_recording():
		return fail("recording did not start")
	for _i in range(45):
		world.step_frame(PackedInt64Array([0]))
	world.stop_recording()
	if not world.save_recording(path):
		return fail("recording did not save")
	if not Box3DRollbackWorld.validate_recording_file(path):
		return fail("recording validation failed")

	var replay := Box3DReplay.new()
	root.add_child(replay)
	if not replay.load_file(path):
		return fail("replay did not load")
	if replay.get_frame_count() != 45:
		return fail("wrong replay frame count")
	for _i in range(10):
		if not replay.step_frame():
			return fail("replay ended early")
	if replay.get_frame() != 10:
		return fail("replay frame did not advance")
	if replay.get_shape_ids().size() != 2:
		return fail("replay shape enumeration failed")
	if replay.get_shape_transforms().size() != 14:
		return fail("replay transform stream failed")
	replay.seek_frame(35)
	if replay.get_frame() != 35:
		return fail("forward seek failed")
	replay.seek_frame(5)
	if replay.get_frame() != 5:
		return fail("backward seek failed")
	if replay.has_diverged():
		return fail("replay diverged")
	replay.close()
	DirAccess.remove_absolute(path)
	print("replay: OK frames=45")
	quit(0)
