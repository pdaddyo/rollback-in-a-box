extends SceneTree

func fail(msg: String) -> void:
	push_error("FAIL: " + msg)
	quit(1)

func build_world() -> Box3DRollbackWorld:
	var world := Box3DRollbackWorld.new()
	root.add_child(world)
	world.set_input_count(2)
	world.create_world()
	world.add_static_box(Vector3(0, -1, 0), Vector3(20, 1, 20))
	for i in range(8):
		var h := world.add_dynamic_box(Vector3(float(i % 4) - 1.5, 2.0 + float(i), float(i / 4)), Vector3(0.35, 0.35, 0.35))
		world.set_body_linear_velocity(h, Vector3(float((i % 3) - 1), 0, float((i % 5) - 2) * 0.25))
	return world

func _initialize() -> void:
	var a := build_world()
	var b := build_world()
	a.init_snapshots(16)
	b.init_snapshots(16)
	a.save_state(0)
	b.save_state(0)
	if a.state_hash() != b.state_hash():
		return fail("initial hashes differ")

	var idle := PackedInt64Array([0, 0])
	var hashes: Array[int] = []
	for f in range(180):
		a.step_frame(idle)
		b.step_frame(idle)
		var ha: int = a.state_hash()
		var hb: int = b.state_hash()
		if ha != hb:
			return fail("lockstep mismatch at frame %d" % f)
		hashes.append(ha)
		if f == 90:
			a.save_state(1)

	if not a.load_state(1):
		return fail("rollback load failed")
	for f in range(91, 180):
		a.step_frame(idle)
		if a.state_hash() != hashes[f]:
			return fail("rollback resim mismatch at frame %d" % f)

	print("determinism: OK")
	quit(0)
