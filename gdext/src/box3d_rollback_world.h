// Box3DRollbackWorld: deterministic Box3D world wrapper for Godot rollback.
//
// This class owns only generic physics state: Box3D world lifetime, primitive
// body creation, fixed stepping, snapshots, hashes, body transforms, and
// recording. Game rules belong in a project-specific simulation node.
#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int64_array.hpp>

#include "box3d/box3d.h"
#include "rollback_shim.h"

#include <vector>

namespace godot {

class Box3DRollbackWorld : public Node3D {
	GDCLASS(Box3DRollbackWorld, Node3D)

public:
	static constexpr float DEFAULT_TIME_STEP = 1.0f / 60.0f;
	static constexpr int DEFAULT_SUB_STEPS = 4;

	enum BodyKind {
		BODY_NONE = -1,
		BODY_STATIC = 0,
		BODY_DYNAMIC = 1,
	};

private:
	struct SideState {
		uint64_t frame = 0;
		std::vector<uint64_t> bodies;
		std::vector<float> body_meta; // 4 per body: x/y/z extents or radius, kind
		bool valid = false;
	};

	struct RollbackScope {
		int64_t target_frame = -1;
		int window = 0;
		int mispredicted_mask = 0;
		int affected_bodies = 0;
		int awake_bodies = 0;
		int total_bodies = 0;
		bool valid = false;
	};

	b3WorldId world_id = b3_nullWorldId;
	B3RollbackCtx *rollback = nullptr;
	std::vector<SideState> side_slots;
	std::vector<std::vector<int>> player_body_handles;
	RollbackScope last_scope;

	uint64_t frame = 0;
	std::vector<uint64_t> bodies;
	std::vector<float> body_meta;

	int worker_count = 1;
	int input_count = 2;
	float time_step = DEFAULT_TIME_STEP;
	int sub_steps = DEFAULT_SUB_STEPS;
	double last_step_time_ms = 0.0;

	b3Recording *recording = nullptr;
	bool recording_active = false;

	int register_body(b3BodyId id, const Vector3 &meta, BodyKind kind);
	uint64_t state_hash_u64() const;

protected:
	static void _bind_methods();

public:
	~Box3DRollbackWorld() override;

	void create_world();
	void destroy_world();
	bool has_world() const;

	int add_static_box(const Vector3 &position, const Vector3 &half_extents, float friction = 0.6f);
	int add_dynamic_box(const Vector3 &position, const Vector3 &half_extents, float density = 300.0f, float friction = 0.6f);
	int add_static_sphere(const Vector3 &position, float radius, float friction = 0.6f);
	int add_dynamic_sphere(const Vector3 &position, float radius, float density = 300.0f, float friction = 0.6f);
	int add_static_capsule(const Vector3 &position, const Vector3 &point_a, const Vector3 &point_b, float radius, float friction = 0.6f);
	int add_dynamic_capsule(const Vector3 &position, const Vector3 &point_a, const Vector3 &point_b, float radius, float density = 300.0f, float friction = 0.6f);

	void set_body_linear_velocity(int handle, const Vector3 &velocity);
	Vector3 get_body_velocity(int handle) const;
	void apply_body_linear_impulse(int handle, const Vector3 &impulse, bool wake = true);

	void step_frame(const PackedInt64Array &inputs);
	uint64_t get_frame() const { return frame; }

	void init_snapshots(int slot_count);
	bool save_state(int slot);
	bool load_state(int slot);
	int64_t state_hash() const { return (int64_t)state_hash_u64(); }

	int get_body_count() const { return (int)bodies.size(); }
	int get_live_body_count() const;
	int get_awake_body_count() const;
	int64_t get_body_id(int handle) const;
	int64_t get_world_id() const;

	// Partial-resimulation scoping. Games declare which bodies each player's
	// input directly influences; the affected set of a rollback is the closure
	// of those seeds over contacts, joints, and swept-AABB proximity for the
	// resimulated window. Static bodies do not propagate the closure.
	void set_player_bodies(int player, const PackedInt64Array &handles);
	PackedInt64Array compute_affected_bodies(int players_mask, int window_frames) const;
	Dictionary get_last_rollback_scope() const;
	PackedFloat32Array get_transforms() const;
	PackedFloat32Array get_body_meta() const;
	Transform3D get_body_transform(int handle) const;

	bool start_recording();
	void stop_recording();
	bool is_recording() const { return recording_active; }
	bool save_recording(const String &path);
	static bool validate_recording_file(const String &path);

	void set_worker_count(int count) { worker_count = count < 1 ? 1 : count; }
	int get_worker_count() const { return worker_count; }
	void set_input_count(int count) { input_count = count < 1 ? 1 : count; }
	int get_input_count() const { return input_count; }
	void set_time_step(float value) { time_step = value > 0.0f ? value : DEFAULT_TIME_STEP; }
	float get_time_step() const { return time_step; }
	void set_sub_steps(int count) { sub_steps = count < 1 ? 1 : count; }
	int get_sub_steps() const { return sub_steps; }
	double get_last_step_time_ms() const { return last_step_time_ms; }

	void rollback_begin(int64_t target_frame, int window_frames, int players_mask);

	bool rollback_has_world() const { return has_world(); }
	int rollback_get_input_count() const { return input_count; }
	void rollback_init_snapshots(int slot_count) { init_snapshots(slot_count); }
	bool rollback_save_state(int slot) { return save_state(slot); }
	bool rollback_load_state(int slot) { return load_state(slot); }
	int64_t rollback_state_hash() const { return state_hash(); }
	void rollback_step_frame(const PackedInt64Array &inputs) { step_frame(inputs); }
};

} // namespace godot
