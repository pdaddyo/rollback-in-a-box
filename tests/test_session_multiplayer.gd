extends SceneTree

const TICKS := 700

func fail(msg: String) -> void:
	push_error("FAIL: " + msg)
	quit(1)

class InputSim:
	extends Node

	var world: Box3DRollbackWorld
	var player_count := 2
	var player_bodies: Array[int] = []
	var side_state := 0
	var side_slots: Array[int] = []

	func setup(p_player_count: int) -> void:
		player_count = p_player_count
		world = Box3DRollbackWorld.new()
		add_child(world)
		world.set_input_count(player_count)
		world.create_world()
		world.add_static_box(Vector3(0, -1, 0), Vector3(40, 1, 40))
		for p in range(player_count):
			player_bodies.append(world.add_dynamic_box(
				Vector3(float(p * 4) - 6.0, 3, 0), Vector3(0.5, 0.5, 0.5)))

	func rollback_has_world() -> bool:
		return world.has_world()

	func rollback_get_input_count() -> int:
		return player_count

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

	func rollback_step_frame(inputs: PackedInt64Array) -> void:
		for p in range(player_count):
			var x := float((inputs[p] & 7) - 3)
			var z := float(((inputs[p] >> 3) & 7) - 3)
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
	var a := (frame / 11 + player * 3) & 15
	var b := (frame / 17 + player * 5) & 15
	return a | (b << 3)

func run_case(name: String, players: int, min_lat: int, max_lat: int, loss: int) -> void:
	var sims: Array[InputSim] = []
	var sessions: Array[Box3DRollbackSession] = []
	# nets[i][j]: link carrying packets from peer i to peer j.
	var nets: Array = []

	for i in range(players):
		var sim := InputSim.new()
		root.add_child(sim)
		sim.setup(players)
		var session := Box3DRollbackSession.new()
		session.set_simulation(sim)
		session.configure(i, players, 2, 8)
		session.start()
		sims.append(sim)
		sessions.append(session)
		var links: Array = []
		for j in range(players):
			links.append(FakeNet.new(1000 + i * 31 + j * 7, min_lat, max_lat, loss))
		nets.append(links)

	for tick in range(TICKS):
		for i in range(players):
			for j in range(players):
				if i == j:
					continue
				for packet in nets[j][i].deliver(tick):
					sessions[i].ingest_packet(packet)
		for i in range(players):
			var input := scripted_input(sessions[i].get_current_frame(), i)
			sessions[i].tick(input)
			var packet := sessions[i].get_packet()
			for j in range(players):
				if i != j:
					nets[i][j].send(packet, tick)
		for i in range(players):
			if sessions[i].is_desynced():
				fail("%s: peer %d desynced at frame %d" % [name, i, sessions[i].get_desync_frame()])

	# Drain with clean delivery so everything confirms.
	for tick in range(TICKS, TICKS + 240):
		for i in range(players):
			for j in range(players):
				if i == j:
					continue
				for packet in nets[j][i].deliver(tick):
					sessions[i].ingest_packet(packet)
		for i in range(players):
			var input := scripted_input(sessions[i].get_current_frame(), i)
			sessions[i].tick(input)
			var packet := sessions[i].get_packet()
			for j in range(players):
				if i != j:
					nets[i][j].queue.append([tick + 1, packet])

	var safe: int = sessions[0].get_safe_frame()
	for i in range(1, players):
		safe = min(safe, sessions[i].get_safe_frame())
	if safe < 100:
		fail("%s: insufficient safe frames %d" % [name, safe])
	var min_frame: int = sessions[0].get_current_frame()
	var max_frame: int = min_frame
	for i in range(1, players):
		min_frame = min(min_frame, sessions[i].get_current_frame())
		max_frame = max(max_frame, sessions[i].get_current_frame())
	if max_frame - min_frame > 32:
		fail("%s: pacing failed, frame spread %d" % [name, max_frame - min_frame])
	var compared := 0
	for f in range(max(0, safe - 60), safe + 1):
		var reference: int = sessions[0].get_hash_for_frame(f)
		if reference == 0:
			continue
		var comparable := true
		for i in range(1, players):
			var h: int = sessions[i].get_hash_for_frame(f)
			if h == 0:
				comparable = false
				break
			if h != reference:
				fail("%s: confirmed hash mismatch at %d (peer %d)" % [name, f, i])
		if comparable:
			compared += 1
	if compared < 20:
		fail("%s: not enough comparable hashes" % name)
	var rollbacks := PackedInt64Array()
	for i in range(players):
		rollbacks.append(sessions[i].get_total_rollback_frames())
	print("%s: OK safe=%d compared=%d rollbacks=%s" % [name, safe, compared, rollbacks])
	for sim in sims:
		sim.queue_free()

func _initialize() -> void:
	run_case("3p clean", 3, 0, 0, 0)
	run_case("3p jitter loss", 3, 2, 8, 10)
	run_case("4p clean", 4, 0, 0, 0)
	run_case("4p jitter loss", 4, 1, 6, 8)
	run_case("4p asymmetric", 4, 0, 12, 5)
	print("session multiplayer: OK")
	quit(0)
