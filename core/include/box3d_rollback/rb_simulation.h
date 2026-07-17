// The rollback simulation contract, engine-neutral.
//
// Box3DRollbackSession (in Godot) historically talked to its simulation through
// seven stringly-typed reflection calls (sim->call("rollback_save_state", ...)).
// rb::RollbackSession instead talks to this abstract interface. Each engine
// provides a thin implementation:
//   - Godot: a bridge that forwards every method to a GDScript Object via call().
//   - Unreal: a bridge that forwards to a UObject implementing a UINTERFACE, or
//     hands rb::RollbackWorld (which implements this directly) straight in.
//   - Pure C++: rb::RollbackWorld implements this interface itself.
#pragma once

#include <cstdint>

namespace rb {

class IRollbackSimulation {
public:
	virtual ~IRollbackSimulation() = default;

	// Whether the underlying simulation object still exists. The Godot bridge
	// returns false once the wrapped Object has been freed; rb::RollbackWorld
	// and other owned sims stay alive for their whole lifetime.
	virtual bool is_alive() const { return true; }

	virtual bool has_world() const = 0;
	virtual int get_input_count() const = 0;
	virtual void init_snapshots(int slot_count) = 0;
	virtual bool save_state(int slot) = 0;
	virtual bool load_state(int slot) = 0;
	virtual uint64_t get_state_hash() const = 0;
	virtual void step_frame(const int64_t *inputs, int count) = 0;

	// Optional partial-resimulation hook. A sim that scopes rollbacks overrides
	// supports_rollback_begin() to return true and implements rollback_begin();
	// the session calls it once at the start of every resimulation.
	virtual bool supports_rollback_begin() const { return false; }
	virtual void rollback_begin(int64_t target_frame, int window_frames, int players_mask) {
		(void)target_frame;
		(void)window_frames;
		(void)players_mask;
	}
};

// Edge events the session emits during tick()/ingest_packet(). Adapters turn
// these into their engine's native eventing: Godot signals, Unreal dynamic
// multicast delegates. Optional — pass null to ignore.
class ISessionObserver {
public:
	virtual ~ISessionObserver() = default;
	virtual void on_desync(int64_t frame) { (void)frame; }
	virtual void on_peer_incompatible(int player, int64_t fingerprint) {
		(void)player;
		(void)fingerprint;
	}
};

} // namespace rb
