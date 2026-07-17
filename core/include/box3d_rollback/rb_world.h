// rb::RollbackWorld — deterministic Box3D world wrapper, engine-neutral.
//
// Extracted (logic-preserving) from the Godot Box3DRollbackWorld. It owns only
// generic physics state: Box3D world lifetime, primitive body creation, fixed
// stepping, snapshots, hashes, body transforms, and recording. Game rules
// belong in a project-specific simulation on top.
//
// RollbackWorld implements rb::IRollbackSimulation directly, so a pure-C++ or
// Unreal caller can hand it straight to rb::RollbackSession. (The Godot adapter
// binds an arbitrary GDScript sim object instead, via a reflection bridge.)
#pragma once

#include <cstdint>
#include <vector>

#include "box3d/box3d.h"

#include "box3d_rollback/rb_math.h"
#include "box3d_rollback/rb_simulation.h"
#include "rollback_shim.h"

namespace rb {

// Plain-struct replacement for the Dictionary the Godot world returned.
struct RollbackScope {
	int64_t target_frame = -1;
	int window = 0;
	int mispredicted_mask = 0;
	int affected_bodies = 0;
	int awake_bodies = 0;
	int total_bodies = 0;
	bool valid = false;
};

class RollbackWorld : public IRollbackSimulation {
public:
	static constexpr float DEFAULT_TIME_STEP = 1.0f / 60.0f;
	static constexpr int DEFAULT_SUB_STEPS = 4;

	enum BodyKind {
		BODY_NONE = -1,
		BODY_STATIC = 0,
		BODY_DYNAMIC = 1,
	};

	RollbackWorld() = default;
	~RollbackWorld() override;

	// Owns raw Box3D handles and a snapshot context; never safe to copy.
	RollbackWorld(const RollbackWorld &) = delete;
	RollbackWorld &operator=(const RollbackWorld &) = delete;

	void create_world();
	void destroy_world();

	int add_static_box(const Vec3 &position, const Vec3 &half_extents, float friction = 0.6f);
	int add_dynamic_box(const Vec3 &position, const Vec3 &half_extents, float density = 300.0f, float friction = 0.6f);
	int add_static_sphere(const Vec3 &position, float radius, float friction = 0.6f);
	int add_dynamic_sphere(const Vec3 &position, float radius, float density = 300.0f, float friction = 0.6f);
	int add_static_capsule(const Vec3 &position, const Vec3 &point_a, const Vec3 &point_b, float radius, float friction = 0.6f);
	int add_dynamic_capsule(const Vec3 &position, const Vec3 &point_a, const Vec3 &point_b, float radius, float density = 300.0f, float friction = 0.6f);

	void set_body_linear_velocity(int handle, const Vec3 &velocity);
	Vec3 get_body_velocity(int handle) const;
	void apply_body_linear_impulse(int handle, const Vec3 &impulse, bool wake = true);

	uint64_t get_frame() const { return frame; }

	int get_body_count() const { return (int)bodies.size(); }
	int get_live_body_count() const;
	int get_awake_body_count() const;
	int64_t get_body_id(int handle) const;
	int64_t get_world_id() const;

	// Partial-resimulation scoping. Games declare which bodies each player's
	// input directly influences; the affected set of a rollback is the closure
	// of those seeds over contacts, joints, and swept-AABB proximity for the
	// resimulated window. Static bodies do not propagate the closure.
	void set_player_bodies(int player, const int64_t *handles, int count);
	std::vector<int64_t> compute_affected_bodies(int players_mask, int window_frames) const;
	const RollbackScope &get_last_rollback_scope() const { return last_scope; }
	std::vector<float> get_transforms() const;
	const std::vector<float> &get_body_meta() const { return body_meta; }
	Transform get_body_transform(int handle) const;

	bool start_recording();
	void stop_recording();
	bool is_recording() const { return recording_active; }
	bool save_recording(const char *path);
	static bool validate_recording_file(const char *path);

	void set_worker_count(int count) { worker_count = count < 1 ? 1 : count; }
	int get_worker_count() const { return worker_count; }
	void set_input_count(int count) { input_count = count < 1 ? 1 : count; }
	void set_time_step(float value) { time_step = value > 0.0f ? value : DEFAULT_TIME_STEP; }
	float get_time_step() const { return time_step; }
	void set_sub_steps(int count) { sub_steps = count < 1 ? 1 : count; }
	int get_sub_steps() const { return sub_steps; }
	double get_last_step_time_ms() const { return last_step_time_ms; }

	// --- rb::IRollbackSimulation ---
	bool has_world() const override;
	int get_input_count() const override { return input_count; }
	void init_snapshots(int slot_count) override;
	bool save_state(int slot) override;
	bool load_state(int slot) override;
	uint64_t get_state_hash() const override { return state_hash_u64(); }
	void step_frame(const int64_t *inputs, int count) override;
	bool supports_rollback_begin() const override { return true; }
	void rollback_begin(int64_t target_frame, int window_frames, int players_mask) override;

private:
	struct SideState {
		uint64_t frame = 0;
		std::vector<uint64_t> bodies;
		std::vector<float> body_meta; // 4 per body: x/y/z extents or radius, kind
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

	int register_body(b3BodyId id, const Vec3 &meta, BodyKind kind);
	uint64_t state_hash_u64() const;
};

} // namespace rb
