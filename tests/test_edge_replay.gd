extends SceneTree

# Edge-case coverage for Box3DReplay loading/seeking/stepping and the
# Box3DRollbackWorld recording API guards. Deterministic, headless-only.

const FIXTURE_FRAMES := 45

var _failed := false
var _fixture_path := ""
var _temp_paths: Array[String] = []

func fail(msg: String) -> void:
	_failed = true
	push_error("FAIL: " + msg)
	quit(1)

func temp_path(file_name: String) -> String:
	var path := ProjectSettings.globalize_path("user://" + file_name)
	if not _temp_paths.has(path):
		_temp_paths.append(path)
	return path

func write_bytes(path: String, bytes: PackedByteArray) -> bool:
	var f := FileAccess.open(path, FileAccess.WRITE)
	if f == null:
		return false
	f.store_buffer(bytes)
	f.close()
	return true

func cleanup() -> void:
	for path in _temp_paths:
		if FileAccess.file_exists(path):
			DirAccess.remove_absolute(path)

# Record the 45-frame fixture used by most cases (same setup as test_replay.gd).
func make_fixture() -> void:
	_fixture_path = temp_path("edge_replay_fixture.b3r")
	var world := Box3DRollbackWorld.new()
	root.add_child(world)
	world.set_input_count(1)
	world.create_world()
	world.add_static_box(Vector3(0, -1, 0), Vector3(10, 1, 10))
	var body := world.add_dynamic_sphere(Vector3(0, 4, 0), 0.5)
	world.set_body_linear_velocity(body, Vector3(1, 0, 0))
	if not world.start_recording():
		return fail("fixture: recording did not start")
	for _i in range(FIXTURE_FRAMES):
		world.step_frame(PackedInt64Array([0]))
	world.stop_recording()
	if not world.save_recording(_fixture_path):
		return fail("fixture: recording did not save")
	if not Box3DRollbackWorld.validate_recording_file(_fixture_path):
		return fail("fixture: recording validation failed")

func load_fixture(replay: Box3DReplay) -> bool:
	if not replay.load_file(_fixture_path):
		fail("fixture did not load")
		return false
	if replay.get_frame_count() != FIXTURE_FRAMES:
		fail("fixture frame count %d != %d" % [replay.get_frame_count(), FIXTURE_FRAMES])
		return false
	return true

func expect_not_loaded(replay: Box3DReplay, label: String) -> void:
	if replay.is_loaded():
		return fail("%s: is_loaded() should be false" % label)
	if replay.get_frame_count() != 0:
		return fail("%s: get_frame_count() should be 0" % label)
	if replay.get_frame() != 0:
		return fail("%s: get_frame() should be 0" % label)

# case: load_bad_files
func case_load_bad_files() -> void:
	var replay := Box3DReplay.new()
	root.add_child(replay)

	# (a) nonexistent path
	var missing := temp_path("edge_replay_does_not_exist.b3r")
	if FileAccess.file_exists(missing):
		DirAccess.remove_absolute(missing)
	if replay.load_file(missing):
		return fail("load_bad_files: nonexistent path loaded")
	expect_not_loaded(replay, "load_bad_files nonexistent")
	if _failed:
		return

	# (b) 256 bytes of garbage (header magic mismatch)
	var garbage_path := temp_path("edge_replay_garbage.b3r")
	var garbage := PackedByteArray()
	garbage.resize(256)
	garbage.fill(0x78) # 'x'
	if not write_bytes(garbage_path, garbage):
		return fail("load_bad_files: could not write garbage file")
	if replay.load_file(garbage_path):
		return fail("load_bad_files: garbage file loaded")
	expect_not_loaded(replay, "load_bad_files garbage")
	if _failed:
		return

	# (c) zero-byte file (smaller than the fixed header)
	var empty_path := temp_path("edge_replay_empty.b3r")
	if not write_bytes(empty_path, PackedByteArray()):
		return fail("load_bad_files: could not write empty file")
	if replay.load_file(empty_path):
		return fail("load_bad_files: empty file loaded")
	expect_not_loaded(replay, "load_bad_files empty")
	if _failed:
		return

	# (d) valid fixture truncated at a few offsets (player deserialization failure)
	var fixture_bytes := FileAccess.get_file_as_bytes(_fixture_path)
	if fixture_bytes.size() < 128:
		return fail("load_bad_files: fixture unexpectedly small")
	var trunc_path := temp_path("edge_replay_truncated.b3r")
	var offsets: Array[int] = [fixture_bytes.size() / 2, fixture_bytes.size() / 4, 64]
	for offset in offsets:
		if not write_bytes(trunc_path, fixture_bytes.slice(0, offset)):
			return fail("load_bad_files: could not write truncated file")
		if replay.load_file(trunc_path):
			return fail("load_bad_files: truncated file (%d bytes) loaded" % offset)
		expect_not_loaded(replay, "load_bad_files truncated %d" % offset)
		if _failed:
			return
		if Box3DRollbackWorld.validate_recording_file(trunc_path):
			return fail("load_bad_files: truncated file (%d bytes) validated" % offset)

# case: seek_out_of_range
func case_seek_out_of_range() -> void:
	var replay := Box3DReplay.new()
	root.add_child(replay)
	if not load_fixture(replay):
		return
	for _i in range(10):
		if not replay.step_frame():
			return fail("seek_out_of_range: fixture ended early")

	replay.seek_frame(-5)
	if replay.get_frame() != 0:
		return fail("seek_out_of_range: negative seek did not clamp to 0")
	if replay.is_at_end():
		return fail("seek_out_of_range: at end after clamped seek to 0")

	replay.seek_frame(1000)
	if replay.get_frame() != FIXTURE_FRAMES:
		return fail("seek_out_of_range: past-end seek stopped at %d" % replay.get_frame())
	if replay.get_frame() != replay.get_frame_count():
		return fail("seek_out_of_range: frame != frame_count after past-end seek")
	if not replay.is_at_end():
		return fail("seek_out_of_range: not at end after past-end seek")
	if replay.step_frame():
		return fail("seek_out_of_range: step_frame succeeded past end")
	replay.close()

# Shared battery for a replay with no player (never loaded, or closed).
func expect_inert(replay: Box3DReplay, label: String) -> void:
	# seek/restart/step just push engine errors; nothing must crash.
	replay.seek_frame(3)
	replay.restart()
	if replay.step_frame():
		return fail("%s: step_frame() should be false" % label)
	if not replay.get_info().is_empty():
		return fail("%s: get_info() should be empty" % label)
	if not replay.get_shape_geometry(0).is_empty():
		return fail("%s: get_shape_geometry(0) should be empty" % label)
	if replay.get_shape_ids().size() != 0:
		return fail("%s: get_shape_ids() should be empty" % label)
	if replay.get_shape_transforms().size() != 0:
		return fail("%s: get_shape_transforms() should be empty" % label)
	if replay.get_frame() != 0:
		return fail("%s: get_frame() should be 0" % label)
	if replay.get_frame_count() != 0:
		return fail("%s: get_frame_count() should be 0" % label)
	if not replay.is_at_end():
		return fail("%s: is_at_end() should be true" % label)
	if replay.has_diverged():
		return fail("%s: has_diverged() should be false" % label)
	if replay.get_diverge_frame() != -1:
		return fail("%s: get_diverge_frame() should be -1" % label)

# case: methods_before_load_and_after_close
func case_methods_before_load_and_after_close() -> void:
	var replay := Box3DReplay.new()
	root.add_child(replay)
	expect_inert(replay, "before load")
	if _failed:
		return
	if not load_fixture(replay):
		return
	replay.close()
	replay.close() # idempotent
	expect_inert(replay, "after close")

# case: step_past_end_exact_count
func case_step_past_end_exact_count() -> void:
	var replay := Box3DReplay.new()
	root.add_child(replay)
	if not load_fixture(replay):
		return
	var stepped := 0
	for _i in range(50):
		if replay.step_frame():
			stepped += 1
	if stepped != FIXTURE_FRAMES:
		return fail("step_past_end: %d true returns, expected %d" % [stepped, FIXTURE_FRAMES])
	if replay.get_frame() != FIXTURE_FRAMES:
		return fail("step_past_end: get_frame() drifted to %d" % replay.get_frame())
	if not replay.is_at_end():
		return fail("step_past_end: not at end after exhausting recording")
	if replay.step_frame():
		return fail("step_past_end: step_frame() true after end")
	if replay.get_frame() != FIXTURE_FRAMES:
		return fail("step_past_end: frame moved after end")
	replay.close()

# case: zero_frame_recording
func case_zero_frame_recording() -> void:
	var path := temp_path("edge_replay_zero_frames.b3r")
	var world := Box3DRollbackWorld.new()
	root.add_child(world)
	world.set_input_count(1)
	world.create_world()
	world.add_static_box(Vector3(0, -1, 0), Vector3(10, 1, 10))
	world.add_dynamic_sphere(Vector3(0, 4, 0), 0.5)
	if not world.start_recording():
		return fail("zero_frame: recording did not start")
	world.stop_recording()
	if not world.save_recording(path):
		return fail("zero_frame: save_recording failed")

	var replay := Box3DReplay.new()
	root.add_child(replay)
	if not replay.load_file(path):
		return fail("zero_frame: load_file failed on zero-frame recording")
	if replay.get_frame_count() != 0:
		return fail("zero_frame: frame count %d != 0" % replay.get_frame_count())
	if replay.get_frame() != 0:
		return fail("zero_frame: frame %d != 0" % replay.get_frame())
	if replay.step_frame():
		return fail("zero_frame: step_frame() true with no frames")
	if not replay.is_at_end():
		return fail("zero_frame: not at end after failed step")
	# Shape enumeration may reflect the seed snapshot; only require no crash.
	replay.get_shape_ids()
	replay.get_shape_transforms()
	replay.close()

# case: reload_and_failed_reload_discard_old_player
func case_reload_and_failed_reload() -> void:
	var replay := Box3DReplay.new()
	root.add_child(replay)
	if not load_fixture(replay):
		return
	for _i in range(10):
		if not replay.step_frame():
			return fail("reload: fixture ended early")

	# (a) reloading the same path resets to frame 0.
	if not replay.load_file(_fixture_path):
		return fail("reload: second load_file failed")
	if not replay.is_loaded():
		return fail("reload: is_loaded() false after reload")
	if replay.get_frame() != 0:
		return fail("reload: frame %d != 0 after reload" % replay.get_frame())
	if replay.get_frame_count() != FIXTURE_FRAMES:
		return fail("reload: frame count wrong after reload")

	# (b) a failed reload discards the previous player (deliberate contract:
	# load_file closes first, so failure leaves the instance unloaded).
	for _i in range(5):
		if not replay.step_frame():
			return fail("reload: fixture ended early before failed reload")
	var missing := temp_path("edge_replay_does_not_exist.b3r")
	if FileAccess.file_exists(missing):
		DirAccess.remove_absolute(missing)
	if replay.load_file(missing):
		return fail("reload: load_file succeeded on missing path")
	if replay.is_loaded():
		return fail("reload: failed reload kept the old player")
	expect_not_loaded(replay, "reload after failed load")

# case: recording_api_guards
func case_recording_api_guards() -> void:
	var world := Box3DRollbackWorld.new()
	root.add_child(world)
	world.set_input_count(1)

	# (a) start_recording before create_world -> false (error pushed by guard).
	if world.start_recording():
		return fail("guards: start_recording succeeded without a world")

	world.create_world()
	world.add_static_box(Vector3(0, -1, 0), Vector3(10, 1, 10))
	world.add_dynamic_sphere(Vector3(0, 4, 0), 0.5)

	# (d) stop_recording with nothing active is a silent no-op.
	world.stop_recording()

	# (b) save_recording before ever recording -> false, and no file created.
	var never_path := temp_path("edge_replay_never_recorded.b3r")
	if FileAccess.file_exists(never_path):
		DirAccess.remove_absolute(never_path)
	if world.save_recording(never_path):
		return fail("guards: save_recording succeeded with no recording")
	if FileAccess.file_exists(never_path):
		return fail("guards: save_recording created a file despite failing")

	# (c) start_recording twice -> second call false, recording still active.
	if not world.start_recording():
		return fail("guards: first start_recording failed")
	if world.start_recording():
		return fail("guards: second start_recording succeeded while active")

	# A normal record/save afterward still works.
	for _i in range(5):
		world.step_frame(PackedInt64Array([0]))
	world.stop_recording()
	var out_path := temp_path("edge_replay_guard_out.b3r")
	if not world.save_recording(out_path):
		return fail("guards: normal save_recording failed after guard calls")
	if not Box3DRollbackWorld.validate_recording_file(out_path):
		return fail("guards: saved recording did not validate")

# case: get_shape_geometry_bogus_ids
func case_shape_geometry_bogus_ids() -> void:
	var replay := Box3DReplay.new()
	root.add_child(replay)
	if not load_fixture(replay):
		return
	for _i in range(5):
		if not replay.step_frame():
			return fail("geometry: fixture ended early")

	var ids := replay.get_shape_ids()
	if ids.size() == 0:
		return fail("geometry: no shape ids on loaded fixture")
	var valid_id := ids[0]

	if not replay.get_shape_geometry(-1).is_empty():
		return fail("geometry: id -1 returned a non-empty dict")
	if not replay.get_shape_geometry(0).is_empty():
		return fail("geometry: id 0 returned a non-empty dict")
	if ids.has(valid_id + 1):
		return fail("geometry: probe id collides with a real shape id")
	if not replay.get_shape_geometry(valid_id + 1).is_empty():
		return fail("geometry: valid_id+1 returned a non-empty dict")

	var geom := replay.get_shape_geometry(valid_id)
	if geom.is_empty():
		return fail("geometry: valid id returned an empty dict")
	if not geom.has("type"):
		return fail("geometry: valid id dict missing 'type'")
	if not geom.has("dynamic"):
		return fail("geometry: valid id dict missing 'dynamic'")
	replay.close()

# case: seek_zero_equivalence_with_restart
func case_seek_zero_equivalence() -> void:
	var a := Box3DReplay.new()
	var b := Box3DReplay.new()
	root.add_child(a)
	root.add_child(b)
	if not load_fixture(a):
		return
	if not load_fixture(b):
		return
	for _i in range(20):
		if not a.step_frame():
			return fail("seek_zero: replay A ended early")
		if not b.step_frame():
			return fail("seek_zero: replay B ended early")

	a.seek_frame(0)
	b.restart()
	if a.get_frame() != 0:
		return fail("seek_zero: seek_frame(0) landed on frame %d" % a.get_frame())
	if b.get_frame() != 0:
		return fail("seek_zero: restart() landed on frame %d" % b.get_frame())

	var ta := a.get_shape_transforms()
	var tb := b.get_shape_transforms()
	if ta.size() == 0:
		return fail("seek_zero: empty transform stream")
	if ta.size() != tb.size():
		return fail("seek_zero: transform stream sizes differ")
	if ta != tb:
		return fail("seek_zero: transforms differ between seek_frame(0) and restart()")
	if a.has_diverged() or b.has_diverged():
		return fail("seek_zero: divergence after backward seek")
	a.close()
	b.close()

func _initialize() -> void:
	make_fixture()
	if _failed:
		return
	var cases: Array[Callable] = [
		case_load_bad_files,
		case_seek_out_of_range,
		case_methods_before_load_and_after_close,
		case_step_past_end_exact_count,
		case_zero_frame_recording,
		case_reload_and_failed_reload,
		case_recording_api_guards,
		case_shape_geometry_bogus_ids,
		case_seek_zero_equivalence,
	]
	for c in cases:
		c.call()
		if _failed:
			return
	cleanup()
	print("edge replay: OK cases=%d" % cases.size())
	quit(0)
