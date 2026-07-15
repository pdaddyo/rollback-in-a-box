extends SceneTree

const TICKS := 500
const SETTLE_FRAMES := 240

func fail(msg: String) -> void:
	push_error("FAIL: " + msg)
	quit(1)

# Simulation with two player bodies near the origin, a prop touching player 0,
# a joint-linked trailer, and a large settled debris field far away.
class ScopedSim:
	extends Node

	var world: Box3DRollbackWorld
	var player_bodies: Array[int] = []
	var prop_near := -1
	var prop_far := -1
	var trailer := -1
	var debris: Array[int] = []
	var side_state := 0
	var side_slots: Array[int] = []

	func setup() -> void:
		world = Box3DRollbackWorld.new()
		add_child(world)
		world.set_input_count(2)
		world.create_world()
		world.add_static_box(Vector3(0, -1, 0), Vector3(80, 1, 80))
		player_bodies.append(world.add_dynamic_box(Vector3(0, 0.5, 0), Vector3(0.5, 0.5, 0.5)))
		player_bodies.append(world.add_dynamic_box(Vector3(0, 0.5, 6), Vector3(0.5, 0.5, 0.5)))
		prop_near = world.add_dynamic_box(Vector3(1.05, 0.5, 0), Vector3(0.5, 0.5, 0.5))
		prop_far = world.add_dynamic_box(Vector3(6, 0.5, -6), Vector3(0.5, 0.5, 0.5))
		# Trailer linked to player 0 by a distance joint, outside AABB reach.
		trailer = world.add_dynamic_box(Vector3(-4, 0.5, 0), Vector3(0.5, 0.5, 0.5))
		var api := Box3D.new()
		var definition: Dictionary = api.b3DefaultDistanceJointDef()
		var base: Dictionary = definition["base"]
		base["bodyIdA"] = world.get_body_id(player_bodies[0])
		base["bodyIdB"] = world.get_body_id(trailer)
		definition["base"] = base
		definition["length"] = 4.0
		definition["minLength"] = 0.0
		definition["maxLength"] = 8.0
		var joint: int = api.b3CreateDistanceJoint(
			world.get_world_id(), api.raw.make_buffer(&"b3DistanceJointDef", definition))
		if not api.b3Joint_IsValid(joint):
			push_error("FAIL: could not create trailer joint")
		# Debris field: 5x5x3 stack far from the players.
		for ix in range(5):
			for iz in range(5):
				for iy in range(3):
					debris.append(world.add_dynamic_box(
						Vector3(40.0 + ix * 1.02, 0.5 + iy * 1.02, 40.0 + iz * 1.02),
						Vector3(0.5, 0.5, 0.5)))
		# Let everything settle and fall asleep before the session starts.
		var idle := PackedInt64Array([0, 0])
		for i in range(SETTLE_FRAMES):
			world.step_frame(idle)
		world.set_player_bodies(0, PackedInt64Array([player_bodies[0]]))
		world.set_player_bodies(1, PackedInt64Array([player_bodies[1]]))

	func rollback_has_world() -> bool:
		return world.has_world()

	func rollback_get_input_count() -> int:
		return 2

	func rollback_init_snapshots(slot_count: int) -> void:
		world.init_snapshots(slot_count)
		side_slots.resize(slot_count)

	func rollback_save_state(slot: int) -> bool:
		if not world.save_state(slot):
			return false
		side_slots[slot] = side_state
		return true

	func rollback_load_state(slot: int) -> bool:
		if not world.load_state(slot):
			return false
		side_state = side_slots[slot]
		return true

	func rollback_state_hash() -> int:
		return world.state_hash() ^ side_state

	func rollback_begin(target_frame: int, window_frames: int, players_mask: int) -> void:
		world.rollback_begin(target_frame, window_frames, players_mask)

	func rollback_step_frame(inputs: PackedInt64Array) -> void:
		for p in range(2):
			# Bounded speed so the players never wander near the debris field.
			var x := float((inputs[p] % 3) - 1)
			var z := float(((inputs[p] / 3) % 3) - 1)
			world.set_body_linear_velocity(player_bodies[p], Vector3(x, 0, z))
			side_state = int((side_state * 1103515245 + inputs[p] * (17 + p) + 12345) & 0x7fffffff)
		world.step_frame(inputs)

class FakeNet:
	var queue: Array = []
	var seed := 1
	var min_lat := 0
	var max_lat := 0
	var loss_pct := 0

	func _init(p_seed: int, p_min_lat: int, p_max_lat: int, p_loss_pct: int) -> void:
		seed = p_seed
		min_lat = p_min_lat
		max_lat = p_max_lat
		loss_pct = p_loss_pct

	func next_rand() -> int:
		seed = int((seed * 1103515245 + 12345) & 0x7fffffff)
		return seed

	func send(packet: PackedByteArray, now: int) -> void:
		if packet.is_empty():
			return
		if loss_pct > 0 and next_rand() % 100 < loss_pct:
			return
		var lat := min_lat
		if max_lat > min_lat:
			lat += next_rand() % (max_lat - min_lat + 1)
		queue.append([now + lat, packet])

	func deliver(now: int) -> Array[PackedByteArray]:
		var out: Array[PackedByteArray] = []
		var keep: Array = []
		for item in queue:
			if item[0] <= now:
				out.append(item[1])
			else:
				keep.append(item)
		queue = keep
		return out

func scripted_input(frame: int, player: int) -> int:
	return ((frame / 13 + player * 2) % 9)

func check_affected_set(sim: ScopedSim) -> void:
	var affected: PackedInt64Array = sim.world.compute_affected_bodies(1, 8)
	var set := {}
	for id in affected:
		set[id] = true
	if not set.has(sim.world.get_body_id(sim.player_bodies[0])):
		fail("affected set missing the seed player body")
	if not set.has(sim.world.get_body_id(sim.prop_near)):
		fail("affected set missing adjacent prop (contact/AABB closure)")
	if not set.has(sim.world.get_body_id(sim.trailer)):
		fail("affected set missing joint-linked trailer")
	if set.has(sim.world.get_body_id(sim.prop_far)):
		fail("affected set wrongly includes distant prop")
	for handle in [sim.debris[0], sim.debris[sim.debris.size() - 1]]:
		if set.has(sim.world.get_body_id(handle)):
			fail("affected set wrongly includes debris")
	# Mask for player 1 only: player 0's entourage must not appear.
	var affected_p1: PackedInt64Array = sim.world.compute_affected_bodies(2, 8)
	var set_p1 := {}
	for id in affected_p1:
		set_p1[id] = true
	if not set_p1.has(sim.world.get_body_id(sim.player_bodies[1])):
		fail("player-1 affected set missing its seed body")
	if set_p1.has(sim.world.get_body_id(sim.trailer)):
		fail("player-1 affected set wrongly includes player 0's trailer")

func _initialize() -> void:
	var sims: Array[ScopedSim] = []
	var sessions: Array[Box3DRollbackSession] = []
	var nets: Array[FakeNet] = []

	for i in range(2):
		var sim := ScopedSim.new()
		root.add_child(sim)
		sim.setup()
		var session := Box3DRollbackSession.new()
		session.set_simulation(sim)
		session.configure(i, 2, 2, 8)
		session.start()
		sims.append(sim)
		sessions.append(session)
		nets.append(FakeNet.new(1000 + i, 2, 8, 10))

	# The debris must actually be asleep for the scope telemetry to mean anything.
	var awake: int = sims[0].world.get_awake_body_count()
	if awake > 12:
		fail("debris did not settle: %d bodies awake" % awake)

	# Affected-set closure is correct and identical on both peers before play.
	check_affected_set(sims[0])
	# Raw body ids embed the per-process world index, so compare across peers
	# by convenience handle instead.
	var handle_sets: Array = []
	for i in range(2):
		var id_to_handle := {}
		for h in range(sims[i].world.get_body_count()):
			id_to_handle[sims[i].world.get_body_id(h)] = h
		var handles := []
		for id in sims[i].world.compute_affected_bodies(3, 8):
			if not id_to_handle.has(id):
				fail("affected set contains unregistered body")
			handles.append(id_to_handle[id])
		handles.sort()
		handle_sets.append(handles)
	if handle_sets[0] != handle_sets[1]:
		fail("affected set differs across identical peers")

	for tick in range(TICKS):
		for i in range(2):
			for packet in nets[i].deliver(tick):
				sessions[i].ingest_packet(packet)
		for i in range(2):
			var input := scripted_input(sessions[i].get_current_frame(), i)
			sessions[i].tick(input)
			nets[1 - i].send(sessions[i].get_packet(), tick)
		for i in range(2):
			if sessions[i].is_desynced():
				fail("peer %d desynced at frame %d" % [i, sessions[i].get_desync_frame()])

	for tick in range(TICKS, TICKS + 180):
		for i in range(2):
			for packet in nets[i].deliver(tick):
				sessions[i].ingest_packet(packet)
		for i in range(2):
			var input := scripted_input(sessions[i].get_current_frame(), i)
			sessions[i].tick(input)
			nets[1 - i].queue.append([tick + 1, sessions[i].get_packet()])

	var rollbacks: int = sessions[0].get_total_rollback_frames() + sessions[1].get_total_rollback_frames()
	if rollbacks < 20:
		fail("network conditions produced too few rollbacks (%d) to exercise scoping" % rollbacks)

	# Confirmed hashes must agree bit-for-bit in the mostly-sleeping world.
	var safe: int = min(sessions[0].get_safe_frame(), sessions[1].get_safe_frame())
	if safe < 100:
		fail("insufficient safe frames %d" % safe)
	var compared := 0
	for f in range(max(0, safe - 60), safe + 1):
		var ha: int = sessions[0].get_hash_for_frame(f)
		var hb: int = sessions[1].get_hash_for_frame(f)
		if ha == 0 or hb == 0:
			continue
		if ha != hb:
			fail("confirmed hash mismatch at %d" % f)
		compared += 1
	if compared < 20:
		fail("not enough comparable hashes")

	# Rollback scope telemetry: rollbacks were driven by remote misprediction
	# and touched a small affected set in a large world.
	var scoped := false
	for i in range(2):
		var scope: Dictionary = sims[i].world.get_last_rollback_scope()
		if not scope["valid"]:
			continue
		scoped = true
		if scope["mispredicted_mask"] == 0:
			fail("rollback scope has empty mispredicted mask")
		if scope["total_bodies"] < 78:
			fail("scope world unexpectedly small: %d" % scope["total_bodies"])
		if scope["affected_bodies"] > 12:
			fail("affected set unexpectedly large: %d of %d" % [scope["affected_bodies"], scope["total_bodies"]])
		if scope["awake_bodies"] > 16:
			fail("awake count unexpectedly large during rollback: %d" % scope["awake_bodies"])
		print("peer %d last scope: affected=%d awake=%d total=%d window=%d mask=%d" % [
			i, scope["affected_bodies"], scope["awake_bodies"], scope["total_bodies"],
			scope["window"], scope["mispredicted_mask"]])
	if not scoped:
		fail("no rollback recorded scope telemetry")

	print("partial resim scoping: OK safe=%d compared=%d rollbacks=%d" % [safe, compared, rollbacks])
	quit(0)
