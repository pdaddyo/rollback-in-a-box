extends SceneTree

func fail(msg: String) -> void:
	push_error("FAIL: " + msg)
	quit(1)

class Sim:
	extends Node

	var world: Box3DRollbackWorld
	var body := -1

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

	func rollback_save_state(slot: int) -> bool:
		return world.save_state(slot)

	func rollback_load_state(slot: int) -> bool:
		return world.load_state(slot)

	func rollback_state_hash() -> int:
		return world.state_hash()

	func rollback_step_frame(inputs: PackedInt64Array) -> void:
		var x := float((inputs[0] & 7) - 3)
		world.set_body_linear_velocity(body, Vector3(x, 0, 0))
		world.step_frame(inputs)

var incompatible_events: Array = []

func _on_peer_incompatible(player: int, fingerprint: int) -> void:
	incompatible_events.append([player, fingerprint])

func _initialize() -> void:
	# Fingerprint: nonzero and stable across calls.
	var fp := Box3DRollbackSession.get_build_fingerprint()
	if fp == 0:
		fail("fingerprint is zero")
	if Box3DRollbackSession.get_build_fingerprint() != fp:
		fail("fingerprint not stable across calls")

	var sims: Array = []
	var sessions: Array = []
	for i in range(2):
		var sim := Sim.new()
		root.add_child(sim)
		sim.setup()
		var session := Box3DRollbackSession.new()
		session.set_simulation(sim)
		session.configure(i, 2, 2, 8)
		session.start()
		sims.append(sim)
		sessions.append(session)

	sessions[1].peer_incompatible.connect(_on_peer_incompatible)

	# Tick peer 0 a few frames and capture a packet.
	for t in range(4):
		sessions[0].tick(t)
	var packet: PackedByteArray = sessions[0].get_packet()
	if packet.size() < 48:
		fail("packet smaller than v2 header: %d" % packet.size())

	# Wire format is explicitly little-endian: magic 'D3RB' = 0x42523344.
	if packet[0] != 0x44 or packet[1] != 0x33 or packet[2] != 0x52 or packet[3] != 0x42:
		fail("packet magic is not little-endian on the wire")
	if packet[4] != 2:
		fail("expected packet version 2, got %d" % packet[4])

	# A corrupted fingerprint must be rejected: no input confirmation, one signal.
	var corrupted := packet.duplicate()
	corrupted[40] = corrupted[40] ^ 0xFF
	sessions[1].ingest_packet(corrupted)
	sessions[1].ingest_packet(corrupted)
	if sessions[1].get_confirmed_frame() >= 0:
		fail("incompatible packet was applied")
	if sessions[1].get_incompatible_peer_mask() != 1:
		fail("incompatible peer mask wrong: %d" % sessions[1].get_incompatible_peer_mask())
	if incompatible_events.size() != 1:
		fail("expected exactly one peer_incompatible signal, got %d" % incompatible_events.size())
	if incompatible_events[0][0] != 0:
		fail("peer_incompatible reported wrong player %d" % incompatible_events[0][0])
	if incompatible_events[0][1] == fp:
		fail("peer_incompatible reported the local fingerprint")

	# The clean packet from the same build must still be accepted.
	sessions[1].ingest_packet(packet)
	if sessions[1].get_confirmed_frame() < 0:
		fail("compatible packet was not applied")

	print("compat: OK fingerprint=%016x" % fp)
	quit(0)
