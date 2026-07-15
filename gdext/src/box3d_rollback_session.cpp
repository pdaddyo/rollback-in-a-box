#include "box3d_rollback_session.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "rollback_shim.h"

#include <cstring>
#include <type_traits>

using namespace godot;

namespace {

// Explicit little-endian byte layout so the wire format does not depend on
// host byte order.
template <typename T>
void write_le(uint8_t *buf, size_t offset, T value) {
	using U = std::make_unsigned_t<T>;
	const U u = (U)value;
	for (size_t i = 0; i < sizeof(T); ++i) {
		buf[offset + i] = (uint8_t)(u >> (8 * i));
	}
}

template <typename T>
T read_le(const uint8_t *buf, size_t offset) {
	using U = std::make_unsigned_t<T>;
	U u = 0;
	for (size_t i = 0; i < sizeof(T); ++i) {
		u |= (U)buf[offset + i] << (8 * i);
	}
	return (T)u;
}

constexpr size_t HEADER_SIZE = 48;

} // namespace

Object *Box3DRollbackSession::live_simulation() const {
	if (simulation == nullptr || simulation_instance_id == 0) {
		return nullptr;
	}
	return ObjectDB::get_instance(ObjectID(simulation_instance_id));
}

bool Box3DRollbackSession::sim_has_world() const {
	Object *sim = live_simulation();
	return sim != nullptr && (bool)sim->call("rollback_has_world");
}

int Box3DRollbackSession::sim_input_count() const {
	Object *sim = live_simulation();
	return sim != nullptr ? (int)sim->call("rollback_get_input_count") : 0;
}

void Box3DRollbackSession::sim_init_snapshots(int slot_count) {
	Object *sim = live_simulation();
	ERR_FAIL_NULL(sim);
	sim->call("rollback_init_snapshots", slot_count);
}

bool Box3DRollbackSession::sim_save_state(int slot) {
	Object *sim = live_simulation();
	return sim != nullptr && (bool)sim->call("rollback_save_state", slot);
}

bool Box3DRollbackSession::sim_load_state(int slot) {
	Object *sim = live_simulation();
	return sim != nullptr && (bool)sim->call("rollback_load_state", slot);
}

uint64_t Box3DRollbackSession::sim_state_hash() const {
	Object *sim = live_simulation();
	if (sim == nullptr) {
		return 0;
	}
	return (uint64_t)(int64_t)sim->call("rollback_state_hash");
}

void Box3DRollbackSession::sim_step_frame(const PackedInt64Array &inputs) {
	Object *sim = live_simulation();
	ERR_FAIL_NULL(sim);
	sim->call("rollback_step_frame", inputs);
}

Box3DRollbackSession::FrameEntry &Box3DRollbackSession::entry_for(int64_t f) {
	FrameEntry &e = ring[f % RING];
	if (e.frame != f) {
		e.frame = f;
		for (int p = 0; p < MAX_PLAYERS; ++p) {
			e.input[p] = 0;
		}
		e.confirmed_mask = 0;
	}
	return e;
}

int64_t Box3DRollbackSession::min_remote_ack() const {
	int64_t m = INT64_MAX;
	for (int p = 0; p < num_players; ++p) {
		if (p == local_player) {
			continue;
		}
		m = MIN(m, remote_ack[p]);
	}
	return m == INT64_MAX ? -1 : m;
}

double Box3DRollbackSession::get_frame_advantage() const {
	double m = 0.0;
	for (int p = 0; p < num_players; ++p) {
		if (p == local_player) {
			continue;
		}
		m = MAX(m, advantage_ema[p]);
	}
	return m;
}

int64_t Box3DRollbackSession::min_remote_confirmed() const {
	int64_t m = INT64_MAX;
	for (int p = 0; p < num_players; ++p) {
		if (p == local_player) {
			continue;
		}
		m = MIN(m, confirmed_frame[p]);
	}
	return m == INT64_MAX ? confirmed_frame[local_player] : m;
}

int64_t Box3DRollbackSession::safe_frame() const {
	int64_t m = confirmed_frame[local_player];
	for (int p = 0; p < num_players; ++p) {
		m = MIN(m, confirmed_frame[p]);
	}
	int64_t sf = m + 1;
	sf = MIN(sf, current_frame);
	return MAX(sf, (int64_t)0);
}

void Box3DRollbackSession::set_simulation(Object *p_simulation) {
	ERR_FAIL_COND(active);
	simulation = p_simulation;
	simulation_instance_id = p_simulation != nullptr ? (uint64_t)p_simulation->get_instance_id() : 0;
}

void Box3DRollbackSession::configure(int p_local_player, int p_num_players, int p_input_delay, int p_max_prediction) {
	ERR_FAIL_COND(active);
	ERR_FAIL_COND(p_num_players < 1 || p_num_players > MAX_PLAYERS);
	ERR_FAIL_COND(p_local_player < 0 || p_local_player >= p_num_players);
	ERR_FAIL_COND(p_input_delay < 0 || p_input_delay > 16);
	ERR_FAIL_COND(p_max_prediction < 1 || p_max_prediction > SNAPSHOT_SLOTS - 8);
	local_player = p_local_player;
	num_players = p_num_players;
	input_delay = p_input_delay;
	max_prediction = p_max_prediction;
}

void Box3DRollbackSession::start() {
	ERR_FAIL_NULL_MSG(live_simulation(), "Box3DRollbackSession: set_simulation() first.");
	ERR_FAIL_COND_MSG(!sim_has_world(), "Box3DRollbackSession: simulation has no active world.");
	ERR_FAIL_COND_MSG(sim_input_count() != num_players,
			"Box3DRollbackSession: simulation input count must match session num_players.");
	ERR_FAIL_COND(active);

	sim_init_snapshots(SNAPSHOT_SLOTS);
	bool ok = sim_save_state(0);
	ERR_FAIL_COND_MSG(!ok, "Box3DRollbackSession: initial snapshot failed.");

	current_frame = 0;
	first_incorrect = -1;
	for (int p = 0; p < MAX_PLAYERS; ++p) {
		confirmed_frame[p] = -1;
		last_input[p] = 0;
	}
	for (int64_t f = 0; f < input_delay; ++f) {
		FrameEntry &e = entry_for(f);
		e.input[local_player] = 0;
		e.confirmed_mask |= (1 << local_player);
	}
	confirmed_frame[local_player] = input_delay - 1;

	hashes[0] = { 0, sim_state_hash() };
	stalled = false;
	desynced = false;
	desync_frame = -1;
	last_rollback_depth = 0;
	total_rollback_frames = 0;
	total_stalled_ticks = 0;
	for (int p = 0; p < MAX_PLAYERS; ++p) {
		remote_current_frame[p] = -1;
		remote_ack[p] = -1;
		remote_hash_frame[p] = -1;
		remote_hash[p] = 0;
		advantage_ema[p] = 0.0;
		remote_advantage[p] = 0.0;
	}
	last_throttle_frame = -1;
	local_fingerprint = b3r_determinism_fingerprint();
	incompatible_mask = 0;
	mispredicted_mask = 0;
	last_mispredicted_mask = 0;
	active = true;
}

int64_t Box3DRollbackSession::get_build_fingerprint() {
	return (int64_t)b3r_determinism_fingerprint();
}

void Box3DRollbackSession::sim_one(int64_t f) {
	FrameEntry &e = entry_for(f);
	PackedInt64Array inputs;
	inputs.resize(num_players);
	for (int p = 0; p < num_players; ++p) {
		if ((e.confirmed_mask & (1 << p)) == 0) {
			e.input[p] = last_input[p];
		}
		inputs[p] = e.input[p];
	}
	sim_step_frame(inputs);
	sim_save_state((int)((f + 1) % SNAPSHOT_SLOTS));
	hashes[(f + 1) % RING] = { f + 1, sim_state_hash() };
}

void Box3DRollbackSession::do_rollback() {
	const int64_t target = first_incorrect;
	last_mispredicted_mask = mispredicted_mask;
	Object *sim = live_simulation();
	if (sim != nullptr && sim->has_method("rollback_begin")) {
		sim->call("rollback_begin", target, (int)(current_frame - target), (int)mispredicted_mask);
	}
	mispredicted_mask = 0;
	bool ok = sim_load_state((int)(target % SNAPSHOT_SLOTS));
	ERR_FAIL_COND_MSG(!ok, "Box3DRollbackSession: snapshot restore failed.");
	last_rollback_depth = (int)(current_frame - target);
	total_rollback_frames += last_rollback_depth;
	for (int64_t f = target; f < current_frame; ++f) {
		sim_one(f);
	}
}

bool Box3DRollbackSession::tick(int64_t local_input) {
	if (!active || live_simulation() == nullptr) {
		return false;
	}
	if (first_incorrect >= 0 && first_incorrect < current_frame) {
		do_rollback();
	} else {
		last_rollback_depth = 0;
	}
	first_incorrect = -1;
	check_remote_hash();
	if (num_players > 1) {
		const int64_t min_conf = min_remote_confirmed();
		if (current_frame - (min_conf + 1) >= max_prediction) {
			stalled = true;
			total_stalled_ticks++;
			return false;
		}
		bool throttle = false;
		for (int p = 0; p < num_players; ++p) {
			if (p != local_player && advantage_ema[p] - remote_advantage[p] > 2.0) {
				throttle = true;
				break;
			}
		}
		if (throttle && current_frame - last_throttle_frame >= 8) {
			last_throttle_frame = current_frame;
			stalled = true;
			total_stalled_ticks++;
			return false;
		}
	}
	stalled = false;
	const int64_t lf = current_frame + input_delay;
	FrameEntry &e = entry_for(lf);
	e.input[local_player] = local_input;
	e.confirmed_mask |= (1 << local_player);
	if (confirmed_frame[local_player] == lf - 1) {
		confirmed_frame[local_player] = lf;
		last_input[local_player] = local_input;
	}
	sim_one(current_frame);
	current_frame++;
	return true;
}

PackedByteArray Box3DRollbackSession::get_packet() {
	PackedByteArray out;
	if (!active) {
		return out;
	}
	const int64_t latest = current_frame + input_delay - 1;
	int64_t start_f = min_remote_ack() + 1;
	start_f = MAX(start_f, latest - (MAX_SEND_INPUTS - 1));
	start_f = MAX(start_f, (int64_t)0);
	int count = latest >= start_f ? (int)(latest - start_f + 1) : 0;
	out.resize((int64_t)HEADER_SIZE + 8 * count);
	uint8_t *w = out.ptrw();
	write_le<uint32_t>(w, 0, PACKET_MAGIC);
	write_le<uint8_t>(w, 4, PACKET_VERSION);
	write_le<uint8_t>(w, 5, (uint8_t)local_player);
	const int adv = (int)CLAMP(get_frame_advantage(), -120.0, 120.0);
	write_le<int8_t>(w, 6, (int8_t)adv);
	write_le<uint8_t>(w, 7, 0);
	write_le<uint32_t>(w, 8, (uint32_t)current_frame);
	const int64_t ack = min_remote_confirmed();
	write_le<uint32_t>(w, 12, ack >= 0 ? (uint32_t)ack : NO_FRAME);
	const int64_t sf = safe_frame();
	const HashEntry &h = hashes[sf % RING];
	if (first_incorrect < 0 && h.frame == sf) {
		write_le<uint32_t>(w, 16, (uint32_t)sf);
		write_le<uint32_t>(w, 20, 0);
		write_le<uint64_t>(w, 24, h.hash);
	} else {
		write_le<uint32_t>(w, 16, NO_FRAME);
		write_le<uint32_t>(w, 20, 0);
		write_le<uint64_t>(w, 24, 0);
	}
	write_le<uint32_t>(w, 32, (uint32_t)start_f);
	write_le<uint16_t>(w, 36, (uint16_t)count);
	write_le<uint16_t>(w, 38, 0);
	write_le<uint64_t>(w, 40, local_fingerprint);
	for (int i = 0; i < count; ++i) {
		const FrameEntry &fe = ring[(start_f + i) % RING];
		const int64_t value = (fe.frame == start_f + i) ? fe.input[local_player] : 0;
		write_le<int64_t>(w, HEADER_SIZE + 8 * (size_t)i, value);
	}
	return out;
}

void Box3DRollbackSession::ingest_packet(const PackedByteArray &packet) {
	if (!active || packet.size() < (int64_t)HEADER_SIZE) {
		return;
	}
	const uint8_t *r = packet.ptr();
	if (read_le<uint32_t>(r, 0) != PACKET_MAGIC || read_le<uint8_t>(r, 4) != PACKET_VERSION) {
		return;
	}
	const int sender = read_le<uint8_t>(r, 5);
	if (sender == local_player || sender >= num_players) {
		return;
	}
	const uint64_t sender_fingerprint = read_le<uint64_t>(r, 40);
	if (sender_fingerprint != local_fingerprint) {
		if ((incompatible_mask & (1 << sender)) == 0) {
			incompatible_mask |= (1 << sender);
			emit_signal("peer_incompatible", sender, (int64_t)sender_fingerprint);
		}
		return;
	}
	const int64_t start_f = (int64_t)read_le<uint32_t>(r, 32);
	const int count = read_le<uint16_t>(r, 36);
	if (packet.size() != (int64_t)(HEADER_SIZE + 8 * (size_t)count)) {
		return;
	}
	const int64_t far_horizon = current_frame + input_delay + RING / 2;
	if (count > MAX_SEND_INPUTS || start_f > far_horizon) {
		return;
	}
	const int64_t their_frame = (int64_t)read_le<uint32_t>(r, 8);
	if (their_frame > far_horizon + RING) {
		return;
	}
	remote_current_frame[sender] = MAX(remote_current_frame[sender], their_frame);
	const uint32_t ack = read_le<uint32_t>(r, 12);
	const int64_t latest_local = current_frame + input_delay - 1;
	if (ack != NO_FRAME && (int64_t)ack <= latest_local) {
		remote_ack[sender] = MAX(remote_ack[sender], (int64_t)ack);
	}
	advantage_ema[sender] = 0.9 * advantage_ema[sender] + 0.1 * (double)(current_frame - their_frame);
	remote_advantage[sender] = (double)read_le<int8_t>(r, 6);
	const uint32_t hash_frame = read_le<uint32_t>(r, 16);
	if (hash_frame != NO_FRAME && (int64_t)hash_frame > remote_hash_frame[sender] && (int64_t)hash_frame <= their_frame) {
		remote_hash_frame[sender] = (int64_t)hash_frame;
		remote_hash[sender] = read_le<uint64_t>(r, 24);
	}
	for (int i = 0; i < count; ++i) {
		const int64_t f = start_f + i;
		if (f <= confirmed_frame[sender]) {
			continue;
		}
		if (f > far_horizon) {
			break;
		}
		const int64_t value = read_le<int64_t>(r, HEADER_SIZE + 8 * (size_t)i);
		FrameEntry &e = entry_for(f);
		const bool already = (e.confirmed_mask & (1 << sender)) != 0;
		if (!already && f < current_frame && e.input[sender] != value) {
			if (first_incorrect < 0 || f < first_incorrect) {
				first_incorrect = f;
			}
			mispredicted_mask |= (uint8_t)(1 << sender);
		}
		e.input[sender] = value;
		e.confirmed_mask |= (1 << sender);
	}
	while (true) {
		const int64_t next = confirmed_frame[sender] + 1;
		const FrameEntry &e = ring[next % RING];
		if (e.frame == next && (e.confirmed_mask & (1 << sender)) != 0) {
			confirmed_frame[sender] = next;
			last_input[sender] = e.input[sender];
		} else {
			break;
		}
	}
	check_remote_hash();
}

void Box3DRollbackSession::check_remote_hash() {
	if (desynced || first_incorrect >= 0) {
		return;
	}
	const int64_t sf = safe_frame();
	for (int p = 0; p < num_players; ++p) {
		if (p == local_player || remote_hash_frame[p] < 0 || remote_hash_frame[p] > sf) {
			continue;
		}
		const HashEntry &h = hashes[remote_hash_frame[p] % RING];
		if (h.frame != remote_hash_frame[p]) {
			continue;
		}
		if (h.hash != remote_hash[p]) {
			desynced = true;
			desync_frame = remote_hash_frame[p];
			emit_signal("desync_detected", remote_hash_frame[p]);
			return;
		}
	}
}

int64_t Box3DRollbackSession::get_hash_for_frame(int64_t frame) const {
	const HashEntry &h = hashes[frame % RING];
	return h.frame == frame ? (int64_t)h.hash : 0;
}

void Box3DRollbackSession::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_simulation", "simulation"), &Box3DRollbackSession::set_simulation);
	ClassDB::bind_method(D_METHOD("configure", "local_player", "num_players", "input_delay", "max_prediction"),
			&Box3DRollbackSession::configure, DEFVAL(2), DEFVAL(2), DEFVAL(8));
	ClassDB::bind_method(D_METHOD("start"), &Box3DRollbackSession::start);
	ClassDB::bind_method(D_METHOD("tick", "local_input"), &Box3DRollbackSession::tick);
	ClassDB::bind_method(D_METHOD("get_packet"), &Box3DRollbackSession::get_packet);
	ClassDB::bind_method(D_METHOD("ingest_packet", "packet"), &Box3DRollbackSession::ingest_packet);
	ClassDB::bind_method(D_METHOD("get_current_frame"), &Box3DRollbackSession::get_current_frame);
	ClassDB::bind_method(D_METHOD("get_confirmed_frame"), &Box3DRollbackSession::get_confirmed_frame);
	ClassDB::bind_method(D_METHOD("get_safe_frame"), &Box3DRollbackSession::get_safe_frame);
	ClassDB::bind_method(D_METHOD("is_stalled"), &Box3DRollbackSession::is_stalled);
	ClassDB::bind_method(D_METHOD("is_desynced"), &Box3DRollbackSession::is_desynced);
	ClassDB::bind_method(D_METHOD("get_desync_frame"), &Box3DRollbackSession::get_desync_frame);
	ClassDB::bind_method(D_METHOD("get_last_rollback_depth"), &Box3DRollbackSession::get_last_rollback_depth);
	ClassDB::bind_method(D_METHOD("get_last_mispredicted_mask"), &Box3DRollbackSession::get_last_mispredicted_mask);
	ClassDB::bind_method(D_METHOD("get_total_rollback_frames"), &Box3DRollbackSession::get_total_rollback_frames);
	ClassDB::bind_method(D_METHOD("get_total_stalled_ticks"), &Box3DRollbackSession::get_total_stalled_ticks);
	ClassDB::bind_method(D_METHOD("get_frame_advantage"), &Box3DRollbackSession::get_frame_advantage);
	ClassDB::bind_method(D_METHOD("get_hash_for_frame", "frame"), &Box3DRollbackSession::get_hash_for_frame);
	ClassDB::bind_method(D_METHOD("get_incompatible_peer_mask"), &Box3DRollbackSession::get_incompatible_peer_mask);
	ClassDB::bind_static_method("Box3DRollbackSession", D_METHOD("get_build_fingerprint"), &Box3DRollbackSession::get_build_fingerprint);
	ADD_SIGNAL(MethodInfo("desync_detected", PropertyInfo(Variant::INT, "frame")));
	ADD_SIGNAL(MethodInfo("peer_incompatible", PropertyInfo(Variant::INT, "player"), PropertyInfo(Variant::INT, "fingerprint")));
}
