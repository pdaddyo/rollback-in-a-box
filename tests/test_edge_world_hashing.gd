extends SceneTree

# Edge-case tests for world state hashing, snapshot round-trips, and setter
# guards on Box3DRollbackWorld. Each case is a bool-returning helper so a
# failure stops the remaining cases; `fail` keeps the repo-wide FAIL: contract.

const IDLE_STEPS_TO_SETTLE := 120

func fail(msg: String) -> void:
	push_error("FAIL: " + msg)
	quit(1)

func failb(msg: String) -> bool:
	fail(msg)
	return false

func idle() -> PackedInt64Array:
	return PackedInt64Array([0, 0])

# Shared deterministic scene: floor plus a few dynamic bodies with velocities.
func build_basic() -> Box3DRollbackWorld:
	var world := Box3DRollbackWorld.new()
	root.add_child(world)
	world.set_input_count(2)
	world.create_world()
	populate_basic(world)
	return world

func populate_basic(world: Box3DRollbackWorld) -> void:
	world.add_static_box(Vector3(0, -1, 0), Vector3(20, 1, 20))
	world.add_dynamic_box(Vector3(0, 4, 0), Vector3(0.5, 0.5, 0.5))
	var s := world.add_dynamic_sphere(Vector3(2, 5, 0), 0.4)
	world.set_body_linear_velocity(s, Vector3(-0.5, 0, 0.25))
	world.add_dynamic_box(Vector3(-2, 6, 1), Vector3(0.35, 0.35, 0.35))

func done(worlds: Array) -> void:
	for w in worlds:
		root.remove_child(w)
		w.free()

# Case 1: an impulse makes hashes diverge; rollback + idle resim converges
# exactly with a world that never saw the impulse (no residue after in-place
# deserialize).
func case_manual_divergent_then_converged() -> bool:
	var a := build_basic()
	var b := build_basic()
	a.init_snapshots(4)
	b.init_snapshots(4)
	if not a.save_state(0) or not b.save_state(0):
		return failb("divergent: save_state failed")
	if a.state_hash() != b.state_hash():
		return failb("divergent: initial hashes differ")

	a.apply_body_linear_impulse(1, Vector3(900, 300, -450))
	for i in range(50):
		a.step_frame(idle())
	for i in range(50):
		b.step_frame(idle())
	if a.state_hash() == b.state_hash():
		return failb("divergent: impulse not observable in hash at frame 50")

	if not a.load_state(0):
		return failb("divergent: load_state failed")
	for i in range(50):
		a.step_frame(idle())
	if a.state_hash() != b.state_hash():
		return failb("divergent: post-rollback resim hash does not match clean world")
	if a.get_transforms() != b.get_transforms():
		return failb("divergent: post-rollback transforms do not match clean world")
	done([a, b])
	return true

# Case 2: save while a settled body is in persistent contact with the floor
# (asserted quiescent via zero velocity at save time), then verify the
# per-frame hash sequence replays exactly after load_state.
func case_restore_mid_contact_and_sleep() -> bool:
	var world := Box3DRollbackWorld.new()
	root.add_child(world)
	world.set_input_count(2)
	world.create_world()
	world.add_static_box(Vector3(0, -1, 0), Vector3(20, 1, 20))
	var box := world.add_dynamic_box(Vector3(0, 3, 0), Vector3(0.5, 0.5, 0.5))
	world.init_snapshots(4)

	for i in range(IDLE_STEPS_TO_SETTLE):
		world.step_frame(idle())
	# The snapshot below is only "mid contact/sleep" if the box has actually
	# settled onto the floor; assert that instead of assuming it.
	if world.get_body_velocity(box) != Vector3.ZERO:
		return failb("mid-contact: box not quiescent at save time (velocity %s)" % world.get_body_velocity(box))
	if not world.save_state(0):
		return failb("mid-contact: save_state failed")
	var h_saved: int = world.state_hash()

	var recorded: Array[int] = []
	for i in range(60):
		world.step_frame(idle())
		recorded.append(world.state_hash())

	if not world.load_state(0):
		return failb("mid-contact: load_state failed")
	if world.get_frame() != IDLE_STEPS_TO_SETTLE:
		return failb("mid-contact: frame not restored")
	if world.state_hash() != h_saved:
		return failb("mid-contact: hash not restored")
	for i in range(60):
		world.step_frame(idle())
		if world.state_hash() != recorded[i]:
			return failb("mid-contact: resim hash mismatch at +%d (contact/sleep state not round-tripped)" % (i + 1))
	done([world])
	return true

# Case 3: many save/step/load cycles on one slot must be a net no-op and must
# not corrupt buffers or the registry view — a control world that never
# snapshotted stays in hash lockstep afterwards.
func case_many_save_load_cycles_same_slot() -> bool:
	var main := build_basic()
	var control := build_basic()
	for i in range(100):
		main.step_frame(idle())
		control.step_frame(idle())
	main.init_snapshots(4)

	var h_pre: int = main.state_hash()
	if h_pre != control.state_hash():
		return failb("cycles: main and control diverged before the loop")

	for i in range(100):
		if not main.save_state(0):
			return failb("cycles: save_state failed at iteration %d" % i)
		main.step_frame(idle())
		if not main.load_state(0):
			return failb("cycles: load_state failed at iteration %d" % i)

	if main.state_hash() != h_pre:
		return failb("cycles: hash drifted after 100 save/step/load cycles")

	for f in range(60):
		main.step_frame(idle())
		control.step_frame(idle())
		if main.state_hash() != control.state_hash():
			return failb("cycles: lost lockstep with control at +%d" % (f + 1))
	done([main, control])
	return true

# Case 4: the frame counter is folded into the hash, so even a fully sleeping
# world hashes differently every frame; load_state reproduces the saved hash.
func case_hash_changes_every_frame_even_when_asleep() -> bool:
	var world := Box3DRollbackWorld.new()
	root.add_child(world)
	world.set_input_count(2)
	world.create_world()
	world.add_static_box(Vector3(0, -1, 0), Vector3(20, 1, 20))
	world.add_dynamic_box(Vector3(0, 2, 0), Vector3(0.5, 0.5, 0.5))
	for i in range(300):
		world.step_frame(idle())

	# The case is only meaningful if the world is actually quiescent: step
	# until the transforms are bitwise stable across consecutive frames.
	var t_ref: PackedFloat32Array = world.get_transforms()
	var stable := false
	for i in range(600):
		world.step_frame(idle())
		var t: PackedFloat32Array = world.get_transforms()
		if t == t_ref:
			stable = true
			break
		t_ref = t
	if not stable:
		return failb("asleep: transforms never stabilized (world not asleep)")

	world.init_snapshots(4)
	if not world.save_state(0):
		return failb("asleep: save_state failed")
	var h_saved: int = world.state_hash()

	var seen: Array[int] = [h_saved]
	for i in range(10):
		world.step_frame(idle())
		if world.get_transforms() != t_ref:
			return failb("asleep: transforms changed at sleeping step %d (world not quiescent)" % (i + 1))
		var h: int = world.state_hash()
		if seen.has(h):
			return failb("asleep: hash repeated at sleeping step %d" % (i + 1))
		seen.append(h)

	if not world.load_state(0):
		return failb("asleep: load_state failed")
	if world.state_hash() != h_saved:
		return failb("asleep: load did not reproduce saved hash exactly")
	done([world])
	return true

# Case 5: +0.0 vs -0.0 in a constructor argument changes body_meta bits and
# therefore the hash — peers must build worlds with byte-identical parameters.
func case_negative_zero_meta_bits_change_hash() -> bool:
	# NOTE: a literal -0.0 argument gets merged with +0.0 in GDScript's script
	# constant pool (they compare equal), so the negation must happen at runtime.
	var pos_zero := 0.0
	var neg_zero: float = -pos_zero
	if not (is_inf(1.0 / neg_zero) and 1.0 / neg_zero < 0.0):
		return failb("negzero: setup failed to produce a runtime -0.0")
	var a := build_nz_world(pos_zero)
	var b := build_nz_world(neg_zero)
	if a.state_hash() == b.state_hash():
		return failb("negzero: -0.0 half-extent bits did not change state_hash")
	done([a, b])
	return true

func build_nz_world(thin_half_y: float) -> Box3DRollbackWorld:
	var world := build_basic()
	# Far away and static: geometry is degenerate (zero thickness) but inert.
	world.add_static_box(Vector3(50, 0, 50), Vector3(1, thin_half_y, 1))
	return world

# Case 6: nonpositive/NaN setter inputs clamp to safe values and the sim still
# runs. NaN > 0.0 is false, so set_time_step(NAN) also falls back to 1/60.
func case_setter_clamps_including_nan_time_step() -> bool:
	var world := Box3DRollbackWorld.new()
	root.add_child(world)
	var default_dt := 1.0 / 60.0
	for bad_dt in [0.0, -1.0, NAN]:
		world.set_time_step(bad_dt)
		if absf(world.get_time_step() - default_dt) > 1e-6:
			return failb("clamps: set_time_step(%s) did not clamp to 1/60 (got %f)" % [bad_dt, world.get_time_step()])
	world.set_sub_steps(0)
	if world.get_sub_steps() != 1:
		return failb("clamps: set_sub_steps(0) did not clamp to 1")
	world.set_sub_steps(-5)
	if world.get_sub_steps() != 1:
		return failb("clamps: set_sub_steps(-5) did not clamp to 1")
	world.set_worker_count(0)
	if world.get_worker_count() != 1:
		return failb("clamps: set_worker_count(0) did not clamp to 1")
	world.set_input_count(-1)
	if world.get_input_count() != 1:
		return failb("clamps: set_input_count(-1) did not clamp to 1")

	world.create_world()
	var box := world.add_dynamic_box(Vector3(0, 5, 0), Vector3(0.5, 0.5, 0.5))
	var y0: float = world.get_body_transform(box).origin.y
	var one_input := PackedInt64Array([0])
	for i in range(10):
		world.step_frame(one_input)
	if world.get_body_transform(box).origin.y >= y0 - 0.01:
		return failb("clamps: box did not fall after clamped setters")
	done([world])
	return true

# Case 7: time_step has no upper clamp — 10.0 is stored verbatim, two worlds
# stay in per-frame hash lockstep, rollback resim reproduces the sequence, and
# transforms stay finite. Numerical sanity of the physics is NOT asserted.
func case_huge_time_step_stays_hash_lockstep() -> bool:
	var a := build_huge_dt_world()
	var b := build_huge_dt_world()
	if a.get_time_step() != 10.0:
		return failb("huge dt: set_time_step(10.0) was not stored verbatim (got %f)" % a.get_time_step())
	a.init_snapshots(4)
	b.init_snapshots(4)

	var hashes: Array[int] = []
	for f in range(20):
		a.step_frame(idle())
		b.step_frame(idle())
		if a.state_hash() != b.state_hash():
			return failb("huge dt: lockstep hash mismatch at frame %d" % (f + 1))
		hashes.append(a.state_hash())
		if f == 9:
			if not a.save_state(0):
				return failb("huge dt: save_state failed")

	if not a.load_state(0):
		return failb("huge dt: load_state failed")
	for f in range(10, 20):
		a.step_frame(idle())
		if a.state_hash() != hashes[f]:
			return failb("huge dt: rollback resim hash mismatch at frame %d" % (f + 1))

	for v in a.get_transforms():
		if not is_finite(v):
			return failb("huge dt: non-finite value found in get_transforms()")
	done([a, b])
	return true

func build_huge_dt_world() -> Box3DRollbackWorld:
	var world := build_basic()
	world.set_time_step(10.0)
	return world

# Case 8: nonpositive radii are rejected before body creation (-1 return) and
# leave no trace: body count and hash match a twin that never made the calls.
# Zero half-extent boxes are deliberately unguarded and not tested here.
func case_nonpositive_radius_rejected_no_state_effect() -> bool:
	var main := build_basic()
	var twin := build_basic()

	# The four calls below intentionally trip ERR_FAIL_COND_V guards; the debug
	# build prints 'ERROR: Condition "radius <= 0.0f" is true' lines. That is
	# the behavior under test, not a failure (run_all.sh only fails on
	# SCRIPT ERROR:/Parse Error:/FAIL:).
	print("radius rejection: the next 4 'ERROR: Condition' lines are expected")
	if main.add_dynamic_sphere(Vector3(0, 10, 0), 0.0) != -1:
		return failb("radius: add_dynamic_sphere(0.0) did not return -1")
	if main.add_dynamic_sphere(Vector3(0, 10, 0), -1.0) != -1:
		return failb("radius: add_dynamic_sphere(-1.0) did not return -1")
	if main.add_static_capsule(Vector3(0, 10, 0), Vector3(0, -0.5, 0), Vector3(0, 0.5, 0), 0.0) != -1:
		return failb("radius: add_static_capsule(radius=0.0) did not return -1")
	if main.add_dynamic_capsule(Vector3(0, 10, 0), Vector3(0, -0.5, 0), Vector3(0, 0.5, 0), -0.5) != -1:
		return failb("radius: add_dynamic_capsule(radius=-0.5) did not return -1")

	if main.get_body_count() != twin.get_body_count():
		return failb("radius: rejected shapes changed get_body_count()")
	if main.state_hash() != twin.state_hash():
		return failb("radius: rejected shapes changed state_hash()")
	done([main, twin])
	return true

# Case 9 (characterization): whether a destroy_world/create_world cycle plus an
# identical rebuild hash-matches a fresh world depends on Box3D id/generation
# reuse. We only record the cross-lifecycle result; the hard assertion is that
# same-lifecycle twins stay in per-frame lockstep.
func case_recreate_world_generation_characterization() -> bool:
	var a := build_basic()
	var a2 := build_basic()
	var b := build_recreated_world()
	var b2 := build_recreated_world()

	var cross_equal: bool = a.state_hash() == b.state_hash()
	print("recreate characterization: fresh vs recreated initial hash %s" % ("EQUAL" if cross_equal else "DIFFERS"))

	for f in range(30):
		a.step_frame(idle())
		a2.step_frame(idle())
		b.step_frame(idle())
		b2.step_frame(idle())
		if a.state_hash() != a2.state_hash():
			return failb("recreate: fresh twins diverged at frame %d" % (f + 1))
		if b.state_hash() != b2.state_hash():
			return failb("recreate: recreated twins diverged at frame %d" % (f + 1))
	done([a, a2, b, b2])
	return true

func build_recreated_world() -> Box3DRollbackWorld:
	var world := Box3DRollbackWorld.new()
	root.add_child(world)
	world.set_input_count(2)
	world.create_world()
	populate_basic(world)
	world.destroy_world()
	world.create_world()
	populate_basic(world)
	return world

func _initialize() -> void:
	if not case_manual_divergent_then_converged():
		return
	if not case_restore_mid_contact_and_sleep():
		return
	if not case_many_save_load_cycles_same_slot():
		return
	if not case_hash_changes_every_frame_even_when_asleep():
		return
	if not case_negative_zero_meta_bits_change_hash():
		return
	if not case_setter_clamps_including_nan_time_step():
		return
	if not case_huge_time_step_stays_hash_lockstep():
		return
	if not case_nonpositive_radius_rejected_no_state_effect():
		return
	if not case_recreate_world_generation_characterization():
		return
	print("edge world hashing: OK cases=9")
	quit(0)
