// Box3DRollbackSession: Godot adapter over the engine-neutral rb::RollbackSession.
//
// This class is now a thin marshalling shim. All rollback/prediction/netcode
// logic lives in core/ (rb::RollbackSession). The adapter's jobs are:
//   - expose the same GDScript-facing API and signals as before,
//   - bridge the game's GDScript simulation Object to rb::IRollbackSimulation
//     (GodotSimBridge, via Object::call reflection — games keep implementing the
//     seven rollback_* methods in GDScript), and
//   - turn core edge events into Godot signals.
// Transport code only forwards the opaque PackedByteArray returned by get_packet().
#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include "box3d_rollback/rb_session.h"
#include "box3d_rollback/rb_simulation.h"

namespace godot {

class Object;

// Forwards rb::IRollbackSimulation calls to a GDScript Object via reflection,
// using a weak instance id so a freed simulation reports itself dead rather than
// dereferencing a dangling pointer.
class GodotSimBridge : public rb::IRollbackSimulation {
	uint64_t instance_id = 0;
	Object *resolve() const;

public:
	void set_object(Object *obj);

	bool is_alive() const override;
	bool has_world() const override;
	int get_input_count() const override;
	void init_snapshots(int slot_count) override;
	bool save_state(int slot) override;
	bool load_state(int slot) override;
	uint64_t get_state_hash() const override;
	void step_frame(const int64_t *inputs, int count) override;
	bool supports_rollback_begin() const override;
	void rollback_begin(int64_t target_frame, int window_frames, int players_mask) override;
};

class Box3DRollbackSession : public RefCounted {
	GDCLASS(Box3DRollbackSession, RefCounted)

	// Turns rb::RollbackSession edge events into Godot signals on the owner.
	struct SignalObserver : public rb::ISessionObserver {
		Box3DRollbackSession *owner = nullptr;
		void on_desync(int64_t frame) override;
		void on_peer_incompatible(int player, int64_t fingerprint) override;
	};

	rb::RollbackSession session;
	GodotSimBridge bridge;
	SignalObserver observer;

protected:
	static void _bind_methods();

public:
	Box3DRollbackSession();

	void set_simulation(Object *p_simulation);
	void configure(int p_local_player, int p_num_players, int p_input_delay = 2, int p_max_prediction = 8);
	void start();

	bool tick(int64_t local_input);
	PackedByteArray get_packet();
	void ingest_packet(const PackedByteArray &packet);

	int64_t get_current_frame() const { return session.get_current_frame(); }
	int64_t get_confirmed_frame() const { return session.get_confirmed_frame(); }
	int64_t get_safe_frame() const { return session.get_safe_frame(); }
	bool is_stalled() const { return session.is_stalled(); }
	bool is_desynced() const { return session.is_desynced(); }
	int64_t get_desync_frame() const { return session.get_desync_frame(); }
	int get_last_rollback_depth() const { return session.get_last_rollback_depth(); }
	int get_last_mispredicted_mask() const { return session.get_last_mispredicted_mask(); }
	int64_t get_total_rollback_frames() const { return session.get_total_rollback_frames(); }
	int64_t get_total_stalled_ticks() const { return session.get_total_stalled_ticks(); }
	double get_frame_advantage() const { return session.get_frame_advantage(); }
	int64_t get_hash_for_frame(int64_t frame) const { return session.get_hash_for_frame(frame); }
	int get_incompatible_peer_mask() const { return session.get_incompatible_peer_mask(); }

	static int64_t get_build_fingerprint();
};

} // namespace godot
