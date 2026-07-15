extends SceneTree

const TICKS := 900

func fail(msg: String) -> void:
	push_error("FAIL: " + msg)
	quit(1)

class InputSim:
	extends Node

	var world: Box3DRollbackWorld
	var body := -1
	var side_state := 0
	var side_slots: Array[int] = []

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
		var x := float((inputs[0] & 7) - 3)
		var z := float(((inputs[1] >> 3) & 7) - 3)
		world.set_body_linear_velocity(body, Vector3(x, 0, z))
		side_state = int((side_state * 1103515245 + inputs[0] * 17 + inputs[1] * 31 + 12345) & 0x7fffffff)
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

func run_case(name: String, min_lat: int, max_lat: int, loss: int) -> void:
	var sims: Array[InputSim] = []
	var sessions: Array[Box3DRollbackSession] = []
	var nets: Array[FakeNet] = []

	for i in range(2):
		var sim := InputSim.new()
		root.add_child(sim)
		sim.setup()
		var session := Box3DRollbackSession.new()
		session.set_simulation(sim)
		session.configure(i, 2, 2, 8)
		session.start()
		sims.append(sim)
		sessions.append(session)
		nets.append(FakeNet.new(1000 + i, min_lat, max_lat, loss))

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
				fail("%s: peer %d desynced at frame %d" % [name, i, sessions[i].get_desync_frame()])

	for tick in range(TICKS, TICKS + 180):
		for i in range(2):
			for packet in nets[i].deliver(tick):
				sessions[i].ingest_packet(packet)
		for i in range(2):
			var input := scripted_input(sessions[i].get_current_frame(), i)
			sessions[i].tick(input)
			nets[1 - i].queue.append([tick + 1, sessions[i].get_packet()])

	var safe: int = min(sessions[0].get_safe_frame(), sessions[1].get_safe_frame())
	if safe < 100:
		fail("%s: insufficient safe frames %d" % [name, safe])
	var compared := 0
	for f in range(max(0, safe - 60), safe + 1):
		var ha: int = sessions[0].get_hash_for_frame(f)
		var hb: int = sessions[1].get_hash_for_frame(f)
		if ha == 0 or hb == 0:
			continue
		if ha != hb:
			fail("%s: confirmed hash mismatch at %d" % [name, f])
		compared += 1
	if compared < 20:
		fail("%s: not enough comparable hashes" % name)
	print("%s: OK safe=%d compared=%d rollbacks=[%d,%d]" % [
		name,
		safe,
		compared,
		sessions[0].get_total_rollback_frames(),
		sessions[1].get_total_rollback_frames(),
	])

func _initialize() -> void:
	run_case("loopback clean", 0, 0, 0)
	run_case("loopback jitter loss", 2, 8, 12)
	print("session loopback: OK")
	quit(0)
