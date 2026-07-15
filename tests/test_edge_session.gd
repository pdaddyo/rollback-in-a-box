extends SceneTree

# Edge-case tests for Box3DRollbackSession: configure/start guards, stall
# behavior, late-input rollback, packet ingest edge cases, lifecycle
# boundaries, and desync detection.
#
# Note: the invalid-call cases below intentionally trip engine ERR_FAIL
# guards, which print plain "ERROR:" lines. Those do not match the
# run_all.sh failure patterns (SCRIPT ERROR:|Parse Error:|FAIL:).

const MAGIC := 0x42523344
const VERSION := 1
const NO_FRAME := 0xFFFFFFFF

var failed := false

func fail(msg: String) -> void:
	failed = true
	push_error("FAIL: " + msg)
	quit(1)

class Sim:
	extends Node

	var world: Box3DRollbackWorld
	var body := -1
	var inputs_count := 2

	func setup(p_inputs: int) -> void:
		inputs_count = p_inputs
		world = Box3DRollbackWorld.new()
		add_child(world)
		world.set_input_count(p_inputs)
		world.create_world()
		world.add_static_box(Vector3(0, -1, 0), Vector3(30, 1, 30))
		body = world.add_dynamic_box(Vector3(0, 3, 0), Vector3(0.5, 0.5, 0.5))

	func rollback_has_world() -> bool:
		return world.has_world()

	func rollback_get_input_count() -> int:
		return inputs_count

	func rollback_init_snapshots(slot_count: int) -> void:
		world.init_snapshots(slot_count)

	func rollback_save_state(slot: int) -> bool:
		return world.save_state(slot)

	func rollback_load_state(slot: int) -> bool:
		return world.load_state(slot)

	func rollback_state_hash() -> int:
		return world.state_hash()

	func rollback_step_frame(inputs: PackedInt64Array) -> void:
		var x := float((inputs[0] & 7) - 3)
		var z := 0.0
		if inputs.size() > 1:
			z = float((inputs[1] & 7) - 3)
		world.set_body_linear_velocity(body, Vector3(x, 0, z))
		world.step_frame(inputs)

# Like Sim, but the reported state hash is perturbed from frame `bad_from`
# onward. The frame counter is part of saved/loaded state so the sim stays
# self-consistent across rollbacks; only its advertised hash differs.
class DesyncSim:
	extends Node

	var world: Box3DRollbackWorld
	var body := -1
	var frame := 0
	var frame_slots: Array[int] = []
	var bad_from := -1

	func setup() -> void:
		world = Box3DRollbackWorld.new()
		add_child(world)
		world.set_input_count(2)
		world.create_world()
		world.add_static_box(Vector3(0, -1, 0), Vector3(30, 1, 30))
		body = world.add_dynamic_box(Vector3(0, 3, 0), Vector3(0.5, 0.5, 0.5))

	func rollback_has_world() -> bool:
		return world.has_world()

	func rollback_get_input_count() -> int:
		return 2

	func rollback_init_snapshots(slot_count: int) -> void:
		world.init_snapshots(slot_count)
		frame_slots.resize(slot_count)

	func rollback_save_state(slot: int) -> bool:
		if not world.save_state(slot):
			return false
		frame_slots[slot] = frame
		return true

	func rollback_load_state(slot: int) -> bool:
		if not world.load_state(slot):
			return false
		frame = frame_slots[slot]
		return true

	func rollback_state_hash() -> int:
		var h: int = world.state_hash()
		if bad_from >= 0 and frame >= bad_from:
			h = h ^ 0x00FF00FF
		return h

	func rollback_step_frame(inputs: PackedInt64Array) -> void:
		var x := float((inputs[0] & 7) - 3)
		var z := float((inputs[1] & 7) - 3)
		world.set_body_linear_velocity(body, Vector3(x, 0, z))
		world.step_frame(inputs)
		frame += 1

class Counter:
	var n := 0

	func bump(_frame: int) -> void:
		n += 1

func make_sim(p_inputs: int) -> Sim:
	var sim := Sim.new()
	root.add_child(sim)
	sim.setup(p_inputs)
	return sim

func make_session(sim: Node, player: int, players: int, delay := 2, pred := 8) -> Box3DRollbackSession:
	var s := Box3DRollbackSession.new()
	s.set_simulation(sim)
	s.configure(player, players, delay, pred)
	s.start()
	return s

# Hand-build a wire packet. ack and hash frame are NO_FRAME so ingesting
# only touches the input frames (plus advantage/remote-frame bookkeeping).
func build_packet(sender: int, their_frame: int, start_f: int, values: Array) -> PackedByteArray:
	var spb := StreamPeerBuffer.new()
	spb.put_u32(MAGIC)
	spb.put_u8(VERSION)
	spb.put_u8(sender)
	spb.put_8(0)
	spb.put_u8(0)
	spb.put_u32(their_frame)
	spb.put_u32(NO_FRAME)
	spb.put_u32(NO_FRAME)
	spb.put_u32(0)
	spb.put_u64(0)
	spb.put_u32(start_f)
	spb.put_u16(values.size())
	spb.put_u16(0)
	for v in values:
		spb.put_64(v)
	return spb.data_array

func snap(s: Box3DRollbackSession) -> Array:
	return [
		s.get_current_frame(),
		s.get_confirmed_frame(),
		s.get_safe_frame(),
		s.get_frame_advantage(),
		s.is_desynced(),
		s.get_total_rollback_frames(),
	]

# Like snap() but without frame advantage: the advantage EMA legitimately
# updates on every received packet (before frame dedup), so it is excluded
# when asserting that re-ingested input frames are no-ops.
func snap_inputs(s: Box3DRollbackSession) -> Array:
	return [
		s.get_current_frame(),
		s.get_confirmed_frame(),
		s.get_safe_frame(),
		s.is_desynced(),
		s.get_total_rollback_frames(),
	]

# Case: configure_guard_rails_preserve_prior_config
func test_configure_guard_rails() -> void:
	var s := Box3DRollbackSession.new()
	s.set_simulation(make_sim(2))
	s.configure(0, 2, 2, 8)
	# Every one of these must ERR_FAIL and leave the valid config intact.
	s.configure(0, 5, 2, 8) # num_players > MAX_PLAYERS
	s.configure(2, 2, 2, 8) # local_player >= num_players
	s.configure(0, 0, 2, 8) # num_players < 1
	s.configure(0, 2, 17, 8) # input_delay > 16
	s.configure(0, 2, 2, 0) # max_prediction < 1
	s.configure(0, 2, 2, 57) # max_prediction > SNAPSHOT_SLOTS - 8
	s.start()
	# Post-start calls rejected by the active guard.
	s.configure(1, 2, 2, 4)
	s.set_simulation(null)
	# If set_simulation(null) had landed, tick would return false immediately.
	# The tick/stall counts pin num_players=2 and max_prediction=8; the packet
	# header decoded below pins local_player=0 (sender byte at offset 5) and
	# input_delay=2 (start_f=0, count = current_frame + input_delay = 10).
	var trues := 0
	for i in range(20):
		if s.tick((i * 3 + 1) & 7):
			trues += 1
	if trues != 8:
		return fail("configure guards: expected 8 free ticks (max_prediction preserved), got %d" % trues)
	if not s.is_stalled():
		return fail("configure guards: session should be stalled after prediction window")
	if s.get_current_frame() != 8:
		return fail("configure guards: current frame %d != 8" % s.get_current_frame())
	if s.get_total_stalled_ticks() != 12:
		return fail("configure guards: stalled ticks %d != 12" % s.get_total_stalled_ticks())
	var pkt := s.get_packet()
	if pkt.size() < 40:
		return fail("configure guards: packet too short (%d bytes)" % pkt.size())
	if pkt.decode_u8(5) != 0:
		return fail("configure guards: packet sender %d != 0 (local_player changed)" % pkt.decode_u8(5))
	if pkt.decode_u32(32) != 0:
		return fail("configure guards: packet start_f %d != 0" % pkt.decode_u32(32))
	if pkt.decode_u16(36) != 10:
		return fail("configure guards: packet input count %d != 10 (input_delay changed)" % pkt.decode_u16(36))

# Case: start_guards
func test_start_guards() -> void:
	var s := Box3DRollbackSession.new()
	s.configure(0, 2, 2, 8)
	s.start() # no simulation set -> ERR_FAIL
	if s.tick(0):
		return fail("start guards: tick succeeded with no simulation")
	if s.get_current_frame() != 0:
		return fail("start guards: frame moved after failed start (no sim)")
	s.set_simulation(make_sim(1)) # reports input count 1, session wants 2
	s.start() # input count mismatch -> ERR_FAIL
	if s.tick(0):
		return fail("start guards: tick succeeded after input-count-mismatch start")
	if s.get_current_frame() != 0:
		return fail("start guards: frame moved after failed start (mismatch)")
	s.set_simulation(make_sim(2))
	s.start()
	if not s.tick(1):
		return fail("start guards: valid start did not activate session")
	s.start() # second start while active -> ERR_FAIL, must not reset anything
	if not s.tick(2):
		return fail("start guards: tick failed after rejected second start")
	if s.get_current_frame() != 2:
		return fail("start guards: current frame %d != 2 after rejected restart" % s.get_current_frame())

# Case: single_peer_never_stalls
func test_single_peer_never_stalls() -> void:
	var s := make_session(make_sim(1), 0, 1)
	for i in range(200):
		if not s.tick((i * 3) & 7):
			return fail("single peer: tick %d returned false" % i)
	if s.is_stalled():
		return fail("single peer: is_stalled() true")
	if s.get_total_stalled_ticks() != 0:
		return fail("single peer: stalled ticks %d != 0" % s.get_total_stalled_ticks())
	if s.get_current_frame() != 200:
		return fail("single peer: current frame %d != 200" % s.get_current_frame())
	# With no remotes, confirmed tracks the local delayed input frame.
	if s.get_confirmed_frame() != 201:
		return fail("single peer: confirmed frame %d != 201" % s.get_confirmed_frame())

# Cases: silent_remote_peer_stalls_at_max_prediction + stall_recovery_and_late_input_rollback
func test_stall_and_recovery() -> void:
	var sa := make_session(make_sim(2), 0, 2)
	var sb := make_session(make_sim(2), 1, 2)
	# A never hears from peer 1: exactly max_prediction=8 free ticks.
	var trues := 0
	for i in range(20):
		if sa.tick(1):
			trues += 1
	if trues != 8:
		return fail("stall: expected 8 free ticks, got %d" % trues)
	if not sa.is_stalled():
		return fail("stall: is_stalled() false while starved")
	if sa.get_current_frame() != 8:
		return fail("stall: current frame %d != 8 (should freeze)" % sa.get_current_frame())
	if sa.get_total_stalled_ticks() != 12:
		return fail("stall: stalled ticks %d != 12" % sa.get_total_stalled_ticks())
	if sa.get_total_rollback_frames() != 0:
		return fail("stall: rollback frames %d != 0 before any packet" % sa.get_total_rollback_frames())
	# Real peer B produces genuine packets: 8 ticks of nonzero input 9,
	# covering input frames 0..9 (frames 0..1 are the zero delay seed).
	for i in range(8):
		if not sb.tick(9):
			return fail("recovery: peer B tick %d failed" % i)
	var pkt := sb.get_packet()
	if pkt.is_empty():
		return fail("recovery: peer B produced empty packet")
	sa.ingest_packet(pkt)
	if sa.get_confirmed_frame() != 9:
		return fail("recovery: confirmed frame %d != 9 after ingest" % sa.get_confirmed_frame())
	# Next tick unstalls and performs the rollback: A predicted 0 for peer 1,
	# first real nonzero input is frame 2, so depth = 8 - 2 = 6.
	if not sa.tick(1):
		return fail("recovery: tick still false after inputs arrived")
	if sa.is_stalled():
		return fail("recovery: is_stalled() still true after recovery tick")
	if sa.get_last_rollback_depth() != 6:
		return fail("recovery: rollback depth %d != 6" % sa.get_last_rollback_depth())
	if sa.get_total_rollback_frames() != 6:
		return fail("recovery: total rollback frames %d != 6" % sa.get_total_rollback_frames())
	if sa.get_current_frame() != 9:
		return fail("recovery: current frame %d != 9" % sa.get_current_frame())
	if sa.get_total_stalled_ticks() != 12:
		return fail("recovery: stalled ticks changed to %d" % sa.get_total_stalled_ticks())

# Case: duplicate_packet_idempotent
func test_duplicate_packet_idempotent() -> void:
	var sa := make_session(make_sim(2), 0, 2)
	var sb := make_session(make_sim(2), 1, 2)
	for i in range(5):
		if not sb.tick(7):
			return fail("dup: peer B tick %d failed" % i)
	var pkt := sb.get_packet() # covers frames 0..6
	sa.ingest_packet(pkt)
	if sa.get_confirmed_frame() != 6:
		return fail("dup: confirmed frame %d != 6 after first ingest" % sa.get_confirmed_frame())
	var s1 := snap_inputs(sa)
	var h1 := sa.get_hash_for_frame(0)
	sa.ingest_packet(pkt)
	if snap_inputs(sa) != s1:
		return fail("dup: immediate re-ingest changed session state")
	if sa.get_hash_for_frame(0) != h1:
		return fail("dup: immediate re-ingest changed hash")
	# Inputs were confirmed before A ever stepped, so no rollback happens.
	for i in range(7):
		if not sa.tick(3):
			return fail("dup: A tick %d failed" % i)
		if sa.get_last_rollback_depth() != 0:
			return fail("dup: unexpected rollback at tick %d" % i)
	var s2 := snap_inputs(sa)
	var h2 := sa.get_hash_for_frame(3)
	sa.ingest_packet(pkt) # third ingest, now entirely below confirmed_frame
	if snap_inputs(sa) != s2:
		return fail("dup: stale re-ingest changed session state")
	if sa.get_hash_for_frame(3) != h2:
		return fail("dup: stale re-ingest changed hash for frame 3")
	if not sa.tick(3):
		return fail("dup: tick failed after stale re-ingest")
	if sa.get_last_rollback_depth() != 0:
		return fail("dup: stale re-ingest caused a rollback")
	if sa.get_total_rollback_frames() != 0:
		return fail("dup: total rollback frames %d != 0" % sa.get_total_rollback_frames())

# Case: conflicting_redelivery_for_confirmed_frames_ignored
func test_conflicting_redelivery_ignored() -> void:
	var sa := make_session(make_sim(2), 0, 2)
	var ref := make_session(make_sim(2), 0, 2)
	var px := build_packet(1, 6, 0, [11, 11, 11, 11, 11, 11])
	sa.ingest_packet(px)
	ref.ingest_packet(px)
	if sa.get_confirmed_frame() != 5:
		return fail("conflict: confirmed frame %d != 5 after X packet" % sa.get_confirmed_frame())
	# Conflicting redelivery for frames <= confirmed_frame[sender]: skipped.
	sa.ingest_packet(build_packet(1, 6, 0, [22, 22, 22, 22, 22, 22]))
	if sa.get_confirmed_frame() != 5:
		return fail("conflict: conflicting packet changed confirmed frame")
	for i in range(6):
		if not sa.tick(4):
			return fail("conflict: A tick %d failed" % i)
		if not ref.tick(4):
			return fail("conflict: reference tick %d failed" % i)
	if sa.get_total_rollback_frames() != 0:
		return fail("conflict: conflicting redelivery caused rollback")
	# Simulation must have used the original X inputs: hashes match a
	# reference session that never saw the conflicting packet.
	for f in range(1, 7):
		var ha: int = sa.get_hash_for_frame(f)
		if ha == 0:
			return fail("conflict: missing hash for frame %d" % f)
		if ha != ref.get_hash_for_frame(f):
			return fail("conflict: inputs were overwritten (hash mismatch at frame %d)" % f)

# Case: far_future_and_horizon_straddling_packets
func test_far_future_and_horizon_straddle() -> void:
	var sa := make_session(make_sim(2), 0, 2)
	# far_horizon = current(0) + delay(2) + RING/2(64) = 66.
	sa.ingest_packet(build_packet(1, 5, 67, [1])) # start_f beyond horizon: dropped whole
	if sa.get_confirmed_frame() != -1:
		return fail("horizon: far-future start_f advanced confirmed frame")
	if sa.get_frame_advantage() != 0.0:
		return fail("horizon: far-future start_f touched frame advantage")
	sa.ingest_packet(build_packet(1, 195, 0, [1])) # their_frame > far_horizon + RING: dropped whole
	if sa.get_confirmed_frame() != -1:
		return fail("horizon: absurd their_frame advanced confirmed frame")
	if sa.get_frame_advantage() != 0.0:
		return fail("horizon: absurd their_frame touched frame advantage")
	if sa.get_current_frame() != 0:
		return fail("horizon: dropped packets moved current frame")
	# Confirm frames 0..63, then a packet straddling the horizon (40..103):
	# only the in-range prefix 64..66 applies, then the loop breaks.
	var base: Array = []
	for i in range(64):
		base.append(7)
	sa.ingest_packet(build_packet(1, 0, 0, base))
	if sa.get_confirmed_frame() != 63:
		return fail("horizon: confirmed frame %d != 63 after full packet" % sa.get_confirmed_frame())
	var over: Array = []
	for i in range(64):
		over.append(8)
	sa.ingest_packet(build_packet(1, 0, 40, over))
	if sa.get_confirmed_frame() != 66:
		return fail("horizon: straddling packet confirmed to %d, expected prefix stop at 66" % sa.get_confirmed_frame())
	if not sa.tick(1):
		return fail("horizon: tick failed after straddling packet")

# Case: out_of_order_gap_blocks_confirmation
func test_out_of_order_gap() -> void:
	var sa := make_session(make_sim(2), 0, 2)
	var ref := make_session(make_sim(2), 0, 2)
	var hi := [5, 5, 5, 5, 5, 5] # frames 10..15
	var lo := [3, 3, 3, 3, 3, 3, 3, 3, 3, 3] # frames 0..9
	sa.ingest_packet(build_packet(1, 16, 10, hi))
	if sa.get_confirmed_frame() != -1:
		return fail("gap: confirmed frame %d != -1 with frames 0..9 missing" % sa.get_confirmed_frame())
	sa.ingest_packet(build_packet(1, 16, 0, lo))
	if sa.get_confirmed_frame() != 15:
		return fail("gap: confirmed frame %d != 15 after gap filled" % sa.get_confirmed_frame())
	# Reference receives everything in order; simulated hashes must match,
	# proving the early out-of-order inputs were kept intact.
	ref.ingest_packet(build_packet(1, 16, 0, lo + hi))
	if ref.get_confirmed_frame() != 15:
		return fail("gap: reference confirmed frame %d != 15" % ref.get_confirmed_frame())
	for i in range(14):
		if not sa.tick(2):
			return fail("gap: A tick %d failed" % i)
		if not ref.tick(2):
			return fail("gap: reference tick %d failed" % i)
	for f in range(1, 15):
		var ha: int = sa.get_hash_for_frame(f)
		if ha == 0:
			return fail("gap: missing hash for frame %d" % f)
		if ha != ref.get_hash_for_frame(f):
			return fail("gap: hash mismatch at frame %d" % f)

# Case: malformed_packets_silently_ignored
func test_malformed_packets_ignored() -> void:
	var sa := make_session(make_sim(2), 0, 2)
	if not sa.tick(1) or not sa.tick(2):
		return fail("malformed: setup ticks failed")
	var before := snap(sa)
	var bads: Array = []
	bads.append(PackedByteArray()) # empty
	var short_pkt := PackedByteArray()
	short_pkt.resize(39) # one byte short of the header
	bads.append(short_pkt)
	var wrong_magic := build_packet(1, 3, 0, [1, 1])
	wrong_magic[0] = 0x00
	bads.append(wrong_magic)
	var wrong_ver := build_packet(1, 3, 0, [1, 1])
	wrong_ver[4] = 9
	bads.append(wrong_ver)
	bads.append(build_packet(0, 3, 0, [1, 1])) # sender == local_player
	bads.append(build_packet(2, 3, 0, [1, 1])) # sender >= num_players
	var mismatch := build_packet(1, 3, 0, [1, 1])
	mismatch[36] = 3 # header says 3 inputs, body holds 2
	bads.append(mismatch)
	var big: Array = []
	for i in range(65):
		big.append(1)
	bads.append(build_packet(1, 3, 0, big)) # count > MAX_SEND_INPUTS
	for i in range(bads.size()):
		sa.ingest_packet(bads[i])
		if snap(sa) != before:
			return fail("malformed: packet case %d changed session state" % i)
	if not sa.tick(3):
		return fail("malformed: tick failed after malformed packets")

# Case: lifecycle_boundaries_tick_before_start_freed_sim_frame0
func test_lifecycle_boundaries() -> void:
	var sim := make_sim(2)
	var s := Box3DRollbackSession.new()
	s.set_simulation(sim)
	s.configure(0, 2, 2, 8)
	# (a) configured but not started
	if s.tick(0):
		return fail("lifecycle: tick returned true before start")
	if not s.get_packet().is_empty():
		return fail("lifecycle: get_packet() not empty before start")
	s.ingest_packet(build_packet(1, 3, 0, [1, 1])) # must be a no-op
	if s.get_current_frame() != 0:
		return fail("lifecycle: pre-start ingest moved current frame")
	if s.get_confirmed_frame() != -1:
		return fail("lifecycle: pre-start ingest changed confirmed frame")
	# (b) immediately after start
	s.start()
	if s.get_current_frame() != 0:
		return fail("lifecycle: current frame %d != 0 at start" % s.get_current_frame())
	if s.get_confirmed_frame() != -1:
		return fail("lifecycle: confirmed frame %d != -1 at start" % s.get_confirmed_frame())
	if s.get_safe_frame() != 0:
		return fail("lifecycle: safe frame %d != 0 at start" % s.get_safe_frame())
	if s.get_hash_for_frame(0) == 0:
		return fail("lifecycle: initial hash for frame 0 is 0")
	if s.get_hash_for_frame(1) != 0:
		return fail("lifecycle: hash for unsimulated frame 1 not 0")
	# (c) simulation freed out from under a started session
	sim.free()
	if s.tick(0):
		return fail("lifecycle: tick returned true after simulation was freed")

# Case: deliberate_desync_detected_once_at_safe_frame
func test_deliberate_desync() -> void:
	const BAD_FROM := 10
	var sim_a := DesyncSim.new()
	root.add_child(sim_a)
	sim_a.setup()
	var sim_b := DesyncSim.new()
	root.add_child(sim_b)
	sim_b.setup()
	sim_b.bad_from = BAD_FROM
	var sa := make_session(sim_a, 0, 2)
	var sb := make_session(sim_b, 1, 2)
	var ca := Counter.new()
	var cb := Counter.new()
	sa.desync_detected.connect(ca.bump)
	sb.desync_detected.connect(cb.bump)
	var pa := PackedByteArray()
	var pb := PackedByteArray()
	for t in range(80):
		if not pb.is_empty():
			sa.ingest_packet(pb)
		if not pa.is_empty():
			sb.ingest_packet(pa)
		if not sa.tick((t * 3 + 1) & 7):
			return fail("desync: A tick %d failed" % t)
		if not sb.tick((t * 5 + 2) & 7):
			return fail("desync: B tick %d failed" % t)
		pa = sa.get_packet()
		pb = sb.get_packet()
	if not sa.is_desynced():
		return fail("desync: A did not detect the perturbed remote hash")
	if not sb.is_desynced():
		return fail("desync: B did not detect the mismatch against clean hashes")
	if ca.n != 1:
		return fail("desync: A emitted desync_detected %d times, expected 1" % ca.n)
	if cb.n != 1:
		return fail("desync: B emitted desync_detected %d times, expected 1" % cb.n)
	if sa.get_desync_frame() < BAD_FROM:
		return fail("desync: A desync frame %d < %d" % [sa.get_desync_frame(), BAD_FROM])
	if sb.get_desync_frame() < BAD_FROM:
		return fail("desync: B desync frame %d < %d" % [sb.get_desync_frame(), BAD_FROM])
	# Nothing stops the session: it keeps ticking, without re-emitting.
	var frame_before: int = sa.get_current_frame()
	for t in range(80, 85):
		sa.ingest_packet(pb)
		sb.ingest_packet(pa)
		if not sa.tick(1):
			return fail("desync: A tick failed after desync")
		if not sb.tick(1):
			return fail("desync: B tick failed after desync")
		pa = sa.get_packet()
		pb = sb.get_packet()
	if sa.get_current_frame() != frame_before + 5:
		return fail("desync: A stopped advancing after desync")
	if ca.n != 1 or cb.n != 1:
		return fail("desync: signal re-emitted after first detection")

func _initialize() -> void:
	var cases := [
		["configure guard rails", test_configure_guard_rails],
		["start guards", test_start_guards],
		["single peer never stalls", test_single_peer_never_stalls],
		["stall and recovery", test_stall_and_recovery],
		["duplicate packet idempotent", test_duplicate_packet_idempotent],
		["conflicting redelivery ignored", test_conflicting_redelivery_ignored],
		["far future and horizon straddle", test_far_future_and_horizon_straddle],
		["out of order gap", test_out_of_order_gap],
		["malformed packets ignored", test_malformed_packets_ignored],
		["lifecycle boundaries", test_lifecycle_boundaries],
		["deliberate desync", test_deliberate_desync],
	]
	for c in cases:
		c[1].call()
		if failed:
			return
		print("edge session [%s]: ok" % c[0])
	print("edge session: OK cases=%d" % cases.size())
	quit(0)
