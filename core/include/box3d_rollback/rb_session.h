// rb::RollbackSession — transport-agnostic, engine-neutral network rollback.
//
// Extracted verbatim (logic-preserving) from the Godot Box3DRollbackSession.
// The session owns prediction, rollback, packet encoding/decoding, acking,
// state-hash exchange, and desync detection. It talks to the game only through
// rb::IRollbackSimulation, and emits edge events through rb::ISessionObserver.
// Transport code only forwards the opaque byte buffer returned by get_packet().
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "box3d_rollback/rb_simulation.h"

namespace rb {

class RollbackSession {
public:
	static constexpr int MAX_PLAYERS = 4;
	static constexpr int RING = 128;
	static constexpr int SNAPSHOT_SLOTS = 64;
	static constexpr int MAX_SEND_INPUTS = 64;
	static constexpr uint32_t PACKET_MAGIC = 0x42523344; // 'D3RB'
	static constexpr uint8_t PACKET_VERSION = 2;
	static constexpr uint32_t NO_FRAME = 0xFFFFFFFF;

	RollbackSession() = default;

	// Holds large per-frame ring buffers and borrowed sim/observer pointers;
	// copying would alias both. Not copyable.
	RollbackSession(const RollbackSession &) = delete;
	RollbackSession &operator=(const RollbackSession &) = delete;

	// The simulation and observer are borrowed, not owned. The caller keeps them
	// alive for the session's active lifetime.
	void set_simulation(IRollbackSimulation *p_simulation);
	void set_observer(ISessionObserver *p_observer) { observer = p_observer; }

	void configure(int p_local_player, int p_num_players, int p_input_delay = 2, int p_max_prediction = 8);
	void start();

	bool tick(int64_t local_input);
	std::vector<uint8_t> get_packet();
	void ingest_packet(const uint8_t *data, size_t size);

	int64_t get_current_frame() const { return current_frame; }
	int64_t get_confirmed_frame() const { return min_remote_confirmed(); }
	int64_t get_safe_frame() const { return safe_frame(); }
	bool is_active() const { return active; }
	bool is_stalled() const { return stalled; }
	bool is_desynced() const { return desynced; }
	int64_t get_desync_frame() const { return desync_frame; }
	int get_last_rollback_depth() const { return last_rollback_depth; }
	int get_last_mispredicted_mask() const { return last_mispredicted_mask; }
	int64_t get_total_rollback_frames() const { return total_rollback_frames; }
	int64_t get_total_stalled_ticks() const { return total_stalled_ticks; }
	double get_frame_advantage() const;
	int64_t get_hash_for_frame(int64_t frame) const;
	int get_incompatible_peer_mask() const { return incompatible_mask; }

	// Determinism fingerprint of this binary. Peers with different fingerprints
	// are rejected at the packet layer; games should also compare fingerprints
	// during matchmaking before starting a session.
	static int64_t get_build_fingerprint();

private:
	struct FrameEntry {
		int64_t frame = -1;
		int64_t input[MAX_PLAYERS] = {};
		uint8_t confirmed_mask = 0;
	};
	struct HashEntry {
		int64_t frame = -1;
		uint64_t hash = 0;
	};

	IRollbackSimulation *simulation = nullptr;
	ISessionObserver *observer = nullptr;
	bool active = false;

	int local_player = 0;
	int num_players = 2;
	int input_delay = 2;
	int max_prediction = 8;

	FrameEntry ring[RING];
	HashEntry hashes[RING];
	int64_t current_frame = 0;
	int64_t confirmed_frame[MAX_PLAYERS] = { -1, -1, -1, -1 };
	int64_t last_input[MAX_PLAYERS] = {};
	int64_t first_incorrect = -1;
	uint8_t mispredicted_mask = 0;
	int last_mispredicted_mask = 0;

	int64_t remote_current_frame[MAX_PLAYERS] = { -1, -1, -1, -1 };
	int64_t remote_ack[MAX_PLAYERS] = { -1, -1, -1, -1 };
	int64_t remote_hash_frame[MAX_PLAYERS] = { -1, -1, -1, -1 };
	uint64_t remote_hash[MAX_PLAYERS] = {};
	double advantage_ema[MAX_PLAYERS] = {};
	double remote_advantage[MAX_PLAYERS] = {};
	int64_t last_throttle_frame = -1;
	uint64_t local_fingerprint = 0;
	uint8_t incompatible_mask = 0;

	bool stalled = false;
	bool desynced = false;
	int64_t desync_frame = -1;
	int last_rollback_depth = 0;
	int64_t total_rollback_frames = 0;
	int64_t total_stalled_ticks = 0;

	bool sim_ok() const { return simulation != nullptr && simulation->is_alive(); }
	FrameEntry &entry_for(int64_t frame);
	int64_t min_remote_confirmed() const;
	int64_t min_remote_ack() const;
	int64_t safe_frame() const;
	bool sim_has_world() const;
	int sim_input_count() const;
	void sim_init_snapshots(int slot_count);
	bool sim_save_state(int slot);
	bool sim_load_state(int slot);
	uint64_t sim_state_hash() const;
	void sim_step_frame(const int64_t *inputs, int count);
	void sim_one(int64_t frame);
	void do_rollback();
	void check_remote_hash();
};

} // namespace rb
