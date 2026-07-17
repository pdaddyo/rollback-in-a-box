#include "box3d_rollback_session.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/packed_int64_array.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <cstring>

using namespace godot;

// --- GodotSimBridge: rb::IRollbackSimulation over a GDScript Object ---

Object *GodotSimBridge::resolve() const {
	if (instance_id == 0) {
		return nullptr;
	}
	return ObjectDB::get_instance(ObjectID(instance_id));
}

void GodotSimBridge::set_object(Object *obj) {
	instance_id = obj != nullptr ? (uint64_t)obj->get_instance_id() : 0;
}

bool GodotSimBridge::is_alive() const {
	return resolve() != nullptr;
}

bool GodotSimBridge::has_world() const {
	Object *sim = resolve();
	return sim != nullptr && (bool)sim->call("rollback_has_world");
}

int GodotSimBridge::get_input_count() const {
	Object *sim = resolve();
	return sim != nullptr ? (int)sim->call("rollback_get_input_count") : 0;
}

void GodotSimBridge::init_snapshots(int slot_count) {
	Object *sim = resolve();
	if (sim != nullptr) {
		sim->call("rollback_init_snapshots", slot_count);
	}
}

bool GodotSimBridge::save_state(int slot) {
	Object *sim = resolve();
	return sim != nullptr && (bool)sim->call("rollback_save_state", slot);
}

bool GodotSimBridge::load_state(int slot) {
	Object *sim = resolve();
	return sim != nullptr && (bool)sim->call("rollback_load_state", slot);
}

uint64_t GodotSimBridge::get_state_hash() const {
	Object *sim = resolve();
	if (sim == nullptr) {
		return 0;
	}
	return (uint64_t)(int64_t)sim->call("rollback_state_hash");
}

void GodotSimBridge::step_frame(const int64_t *inputs, int count) {
	Object *sim = resolve();
	if (sim == nullptr) {
		return;
	}
	PackedInt64Array arr;
	arr.resize(count);
	for (int i = 0; i < count; ++i) {
		arr[i] = inputs[i];
	}
	sim->call("rollback_step_frame", arr);
}

bool GodotSimBridge::supports_rollback_begin() const {
	Object *sim = resolve();
	return sim != nullptr && sim->has_method("rollback_begin");
}

void GodotSimBridge::rollback_begin(int64_t target_frame, int window_frames, int players_mask) {
	Object *sim = resolve();
	if (sim != nullptr) {
		sim->call("rollback_begin", target_frame, window_frames, players_mask);
	}
}

// --- Box3DRollbackSession: Godot class over rb::RollbackSession ---

void Box3DRollbackSession::SignalObserver::on_desync(int64_t frame) {
	if (owner != nullptr) {
		owner->emit_signal("desync_detected", frame);
	}
}

void Box3DRollbackSession::SignalObserver::on_peer_incompatible(int player, int64_t fingerprint) {
	if (owner != nullptr) {
		owner->emit_signal("peer_incompatible", player, fingerprint);
	}
}

Box3DRollbackSession::Box3DRollbackSession() {
	observer.owner = this;
	session.set_simulation(&bridge);
	session.set_observer(&observer);
}

void Box3DRollbackSession::set_simulation(Object *p_simulation) {
	if (session.is_active()) {
		return;
	}
	bridge.set_object(p_simulation);
}

void Box3DRollbackSession::configure(int p_local_player, int p_num_players, int p_input_delay, int p_max_prediction) {
	session.configure(p_local_player, p_num_players, p_input_delay, p_max_prediction);
}

void Box3DRollbackSession::start() {
	session.start();
}

int64_t Box3DRollbackSession::get_build_fingerprint() {
	return rb::RollbackSession::get_build_fingerprint();
}

bool Box3DRollbackSession::tick(int64_t local_input) {
	return session.tick(local_input);
}

PackedByteArray Box3DRollbackSession::get_packet() {
	const std::vector<uint8_t> bytes = session.get_packet();
	PackedByteArray out;
	out.resize((int64_t)bytes.size());
	if (!bytes.empty()) {
		std::memcpy(out.ptrw(), bytes.data(), bytes.size());
	}
	return out;
}

void Box3DRollbackSession::ingest_packet(const PackedByteArray &packet) {
	session.ingest_packet(packet.ptr(), (size_t)packet.size());
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
