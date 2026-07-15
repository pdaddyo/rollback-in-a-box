// Box3DRollbackSession: transport-agnostic network rollback session.
//
// The session owns prediction, rollback, packet encoding/decoding, acking,
// state-hash exchange, and desync detection. Transport code only forwards the
// opaque PackedByteArray returned by get_packet().
#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

namespace godot {

class Object;

class Box3DRollbackSession : public RefCounted {
	GDCLASS(Box3DRollbackSession, RefCounted)

public:
	static constexpr int MAX_PLAYERS = 4;
	static constexpr int RING = 128;
	static constexpr int SNAPSHOT_SLOTS = 64;
	static constexpr int MAX_SEND_INPUTS = 64;
	static constexpr uint32_t PACKET_MAGIC = 0x42523344; // 'D3RB'
	static constexpr uint8_t PACKET_VERSION = 1;
	static constexpr uint32_t NO_FRAME = 0xFFFFFFFF;

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

	Object *simulation = nullptr;
	uint64_t simulation_instance_id = 0;
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

	int64_t remote_current_frame = -1;
	int64_t remote_ack = -1;
	int64_t remote_hash_frame = -1;
	uint64_t remote_hash = 0;
	double advantage_ema = 0.0;
	double remote_advantage = 0.0;
	int64_t last_throttle_frame = -1;

	bool stalled = false;
	bool desynced = false;
	int64_t desync_frame = -1;
	int last_rollback_depth = 0;
	int64_t total_rollback_frames = 0;
	int64_t total_stalled_ticks = 0;

	Object *live_simulation() const;
	FrameEntry &entry_for(int64_t frame);
	int64_t min_remote_confirmed() const;
	int64_t safe_frame() const;
	bool sim_has_world() const;
	int sim_input_count() const;
	void sim_init_snapshots(int slot_count);
	bool sim_save_state(int slot);
	bool sim_load_state(int slot);
	uint64_t sim_state_hash() const;
	void sim_step_frame(const PackedInt64Array &inputs);
	void sim_one(int64_t frame);
	void do_rollback();
	void check_remote_hash();

protected:
	static void _bind_methods();

public:
	void set_simulation(Object *p_simulation);
	void configure(int p_local_player, int p_num_players, int p_input_delay = 2, int p_max_prediction = 8);
	void start();

	bool tick(int64_t local_input);
	PackedByteArray get_packet();
	void ingest_packet(const PackedByteArray &packet);

	int64_t get_current_frame() const { return current_frame; }
	int64_t get_confirmed_frame() const { return min_remote_confirmed(); }
	int64_t get_safe_frame() const { return safe_frame(); }
	bool is_stalled() const { return stalled; }
	bool is_desynced() const { return desynced; }
	int64_t get_desync_frame() const { return desync_frame; }
	int get_last_rollback_depth() const { return last_rollback_depth; }
	int64_t get_total_rollback_frames() const { return total_rollback_frames; }
	int64_t get_total_stalled_ticks() const { return total_stalled_ticks; }
	double get_frame_advantage() const { return advantage_ema; }
	int64_t get_hash_for_frame(int64_t frame) const;
};

} // namespace godot
