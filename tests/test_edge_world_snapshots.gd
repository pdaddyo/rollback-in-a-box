extends SceneTree

# Edge cases for Box3DRollbackWorld snapshot/lifecycle behavior.
# Intentionally triggers native ERR_FAIL guards; those print "ERROR:" lines
# (not "SCRIPT ERROR:"), which run_all.sh does not treat as failure.

const IDLE := [0, 0]

func fail(msg: String) -> void:
	push_error("FAIL: " + msg)
	quit(1)

func make_world() -> Box3DRollbackWorld:
	var world := Box3DRollbackWorld.new()
	root.add_child(world)
	world.set_input_count(2)
	world.create_world()
	return world

func make_populated_world() -> Box3DRollbackWorld:
	var world := make_world()
	world.add_static_box(Vector3(0, -1, 0), Vector3(20, 1, 20))
	var box := world.add_dynamic_box(Vector3(0, 4, 0), Vector3(0.5, 0.5, 0.5))
	world.add_dynamic_sphere(Vector3(2, 5, 0), 0.4)
	world.set_body_linear_velocity(box, Vector3(1, 0, 0))
	return world

func idle() -> PackedInt64Array:
	return PackedInt64Array(IDLE)

func case_save_load_before_init_snapshots() -> bool:
	var world := make_populated_world()
	var frame0: int = world.get_frame()
	var h0: int = world.state_hash()
	if world.save_state(0):
		fail("save_state before init_snapshots returned true")
		return false
	if world.load_state(0):
		fail("load_state before init_snapshots returned true")
		return false
	if world.get_frame() != frame0:
		fail("frame changed by save/load before init_snapshots")
		return false
	if world.state_hash() != h0:
		fail("hash changed by save/load before init_snapshots")
		return false
	return true

func case_load_valid_slot_never_saved() -> bool:
	var world := make_populated_world()
	world.init_snapshots(4)
	var frame0: int = world.get_frame()
	var h0: int = world.state_hash()
	var count0: int = world.get_body_count()
	if world.load_state(0):
		fail("load_state of never-saved slot returned true")
		return false
	if world.get_frame() != frame0:
		fail("frame changed by load of never-saved slot")
		return false
	if world.state_hash() != h0:
		fail("hash changed by load of never-saved slot")
		return false
	if world.get_body_count() != count0:
		fail("body count changed by load of never-saved slot")
		return false
	return true

func case_out_of_range_and_negative_slots() -> bool:
	var world := make_populated_world()
	world.init_snapshots(4)
	var frame0: int = world.get_frame()
	var h0: int = world.state_hash()
	if world.save_state(4):
		fail("save_state(4) with 4 slots returned true")
		return false
	if world.save_state(-1):
		fail("save_state(-1) returned true")
		return false
	if world.load_state(4):
		fail("load_state(4) with 4 slots returned true")
		return false
	if world.load_state(-1):
		fail("load_state(-1) returned true")
		return false
	if world.get_frame() != frame0 or world.state_hash() != h0:
		fail("world state changed by out-of-range slot calls")
		return false
	return true

func case_init_snapshots_zero_or_negative() -> bool:
	# (a) init_snapshots(0) on a fresh world errors before allocating anything,
	# so save_state(0) still fails with the "call init_snapshots() first" guard.
	var a := make_populated_world()
	a.init_snapshots(0)
	if a.save_state(0):
		fail("save_state(0) after init_snapshots(0) returned true")
		return false
	# (a2) init_snapshots(-1) likewise errors before allocating anything
	# (ERR_FAIL_COND(slot_count <= 0)), so save_state(0) still fails.
	var n := make_populated_world()
	n.init_snapshots(-1)
	if n.save_state(0):
		fail("save_state(0) after init_snapshots(-1) returned true")
		return false
	# (b) init_snapshots(0) after a valid init is a no-op (early ERR_FAIL before
	# the rollback ctx is rebuilt), so a prior save survives it.
	var b := make_populated_world()
	b.init_snapshots(4)
	if not b.save_state(0):
		fail("save_state(0) after valid init_snapshots(4) failed")
		return false
	b.init_snapshots(0)
	if not b.load_state(0):
		fail("load_state(0) failed after no-op init_snapshots(0)")
		return false
	# (b2) init_snapshots(-1) after a valid init is likewise a no-op.
	b.init_snapshots(-1)
	if not b.load_state(0):
		fail("load_state(0) failed after no-op init_snapshots(-1)")
		return false
	return true

func case_reinit_snapshots_invalidates_prior_saves() -> bool:
	var world := make_populated_world()
	world.init_snapshots(4)
	if not world.save_state(0):
		fail("initial save_state(0) failed")
		return false
	for i in range(5):
		world.step_frame(idle())
	world.init_snapshots(4)
	var frame_pre: int = world.get_frame()
	var h_pre: int = world.state_hash()
	if world.load_state(0):
		fail("load_state(0) after re-init returned true")
		return false
	if world.get_frame() != frame_pre or world.state_hash() != h_pre:
		fail("world state changed by failed load after re-init")
		return false
	if not world.save_state(0):
		fail("fresh save_state(0) after re-init failed")
		return false
	if not world.load_state(0):
		fail("fresh load_state(0) after re-init failed")
		return false
	if world.get_frame() != frame_pre or world.state_hash() != h_pre:
		fail("fresh save/load cycle after re-init did not round-trip")
		return false
	return true

func case_restore_removes_bodies_added_post_save() -> bool:
	var world := make_populated_world()
	var control := make_populated_world()
	world.init_snapshots(2)
	if not world.save_state(0):
		fail("save_state(0) failed in restore test")
		return false
	var frame_saved: int = world.get_frame()
	var count_saved: int = world.get_body_count()
	var h_saved: int = world.state_hash()
	if control.state_hash() != h_saved:
		fail("control world hash differs before divergence")
		return false
	world.add_dynamic_box(Vector3(0, 8, 0), Vector3(0.3, 0.3, 0.3))
	world.add_dynamic_box(Vector3(1, 9, 0), Vector3(0.3, 0.3, 0.3))
	for i in range(10):
		world.step_frame(idle())
	if not world.load_state(0):
		fail("load_state(0) failed in restore test")
		return false
	if world.get_body_count() != count_saved:
		fail("body count not restored after load (extra bodies survived)")
		return false
	if world.get_frame() != frame_saved:
		fail("frame not restored after load in restore test")
		return false
	if world.state_hash() != h_saved:
		fail("hash not restored after load in restore test")
		return false
	for f in range(20):
		world.step_frame(idle())
		control.step_frame(idle())
		if world.state_hash() != control.state_hash():
			fail("post-restore lockstep mismatch with control at frame %d" % f)
			return false
	return true

func case_destroy_world_lifecycle() -> bool:
	var world := make_populated_world()
	world.init_snapshots(4)
	if not world.save_state(0):
		fail("save_state(0) failed before destroy_world")
		return false
	world.destroy_world()
	if world.state_hash() != 0:
		fail("state_hash after destroy_world is not 0")
		return false
	if world.get_body_count() != 0:
		fail("body count after destroy_world is not 0")
		return false
	world.step_frame(idle())
	if world.get_frame() != 0:
		fail("frame advanced by step_frame after destroy_world")
		return false
	world.create_world()
	if world.load_state(0):
		fail("load_state(0) succeeded after world recreation (snapshots must not survive)")
		return false
	return true

func case_step_frame_wrong_input_count_is_noop() -> bool:
	var world := make_populated_world()
	var wrong_inputs: Array = [
		PackedInt64Array([0]),
		PackedInt64Array(),
		PackedInt64Array([0, 0, 0]),
	]
	for inputs in wrong_inputs:
		var frame0: int = world.get_frame()
		var h0: int = world.state_hash()
		world.step_frame(inputs)
		if world.get_frame() != frame0:
			fail("frame advanced by step_frame with %d inputs" % inputs.size())
			return false
		if world.state_hash() != h0:
			fail("hash changed by step_frame with %d inputs" % inputs.size())
			return false
	var frame_before: int = world.get_frame()
	world.step_frame(idle())
	if world.get_frame() != frame_before + 1:
		fail("correct-size step_frame did not advance frame by 1")
		return false
	return true

func case_empty_world_snapshot_and_hash() -> bool:
	var a := make_world()
	var b := make_world()
	var ha: int = a.state_hash()
	if a.state_hash() != ha:
		fail("empty world hash not stable across calls")
		return false
	if b.state_hash() != ha:
		fail("identical empty worlds hash differently")
		return false
	a.init_snapshots(2)
	if not a.save_state(0):
		fail("save_state(0) on empty world failed")
		return false
	a.step_frame(idle())
	if a.get_frame() != 1:
		fail("empty world frame did not advance")
		return false
	if a.state_hash() == ha:
		fail("empty world hash unchanged after stepping (frame should be hashed)")
		return false
	if not a.load_state(0):
		fail("load_state(0) on empty world failed")
		return false
	if a.get_frame() != 0:
		fail("empty world frame not restored")
		return false
	if a.state_hash() != ha:
		fail("empty world hash not restored")
		return false
	return true

func case_repeated_load_same_slot_idempotent() -> bool:
	var world := make_world()
	world.add_static_box(Vector3(0, -1, 0), Vector3(20, 1, 20))
	var box := world.add_dynamic_box(Vector3(0, 4, 0), Vector3(0.5, 0.5, 0.5))
	world.set_body_linear_velocity(box, Vector3(1, 0, 0))
	world.init_snapshots(4)
	if not world.save_state(0):
		fail("save_state(0) failed in idempotent-load test")
		return false
	for i in range(30):
		world.step_frame(idle())
	if not world.load_state(0):
		fail("first load_state(0) failed")
		return false
	var h: int = world.state_hash()
	if world.get_frame() != 0:
		fail("first load did not restore frame 0")
		return false
	if not world.load_state(0):
		fail("second load_state(0) failed")
		return false
	if world.get_frame() != 0 or world.state_hash() != h:
		fail("second load of same slot not idempotent")
		return false
	for i in range(5):
		world.step_frame(idle())
	if not world.load_state(0):
		fail("third load_state(0) failed")
		return false
	if world.get_frame() != 0 or world.state_hash() != h:
		fail("third load of same slot not idempotent")
		return false
	world.set_body_linear_velocity(box, Vector3(3, 0, 0))
	if world.get_body_velocity(box) != Vector3(3, 0, 0):
		fail("pre-save body handle unusable after repeated loads")
		return false
	return true

func _initialize() -> void:
	var cases: Array = [
		["save_load_before_init_snapshots", case_save_load_before_init_snapshots],
		["load_valid_slot_never_saved", case_load_valid_slot_never_saved],
		["out_of_range_and_negative_slots", case_out_of_range_and_negative_slots],
		["init_snapshots_zero_or_negative", case_init_snapshots_zero_or_negative],
		["reinit_snapshots_invalidates_prior_saves", case_reinit_snapshots_invalidates_prior_saves],
		["restore_removes_bodies_added_post_save", case_restore_removes_bodies_added_post_save],
		["destroy_world_lifecycle", case_destroy_world_lifecycle],
		["step_frame_wrong_input_count_is_noop", case_step_frame_wrong_input_count_is_noop],
		["empty_world_snapshot_and_hash", case_empty_world_snapshot_and_hash],
		["repeated_load_same_slot_idempotent", case_repeated_load_same_slot_idempotent],
	]
	for entry in cases:
		var case_fn: Callable = entry[1]
		if not case_fn.call():
			return
	print("edge world snapshots: OK cases=%d" % cases.size())
	quit(0)
