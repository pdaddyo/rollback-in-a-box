// Box3DRollbackWorld: Godot adapter over the engine-neutral rb::RollbackWorld.
//
// A thin Node3D wrapper. All physics/snapshot/hash logic lives in core/
// (rb::RollbackWorld). This class marshals Godot types (Vector3, Transform3D,
// Packed*Array, Dictionary, String) to/from the neutral core and preserves the
// exact GDScript-facing API — including the rollback_* methods a game sim node
// forwards to, and which Box3DRollbackSession's reflection bridge calls.
#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int64_array.hpp>

#include "box3d_rollback/rb_world.h"

namespace godot {

class Box3DRollbackWorld : public Node3D {
	GDCLASS(Box3DRollbackWorld, Node3D)

public:
	enum BodyKind {
		BODY_NONE = -1,
		BODY_STATIC = 0,
		BODY_DYNAMIC = 1,
	};

private:
	rb::RollbackWorld world;

protected:
	static void _bind_methods();

public:
	void create_world() { world.create_world(); }
	void destroy_world() { world.destroy_world(); }
	bool has_world() const { return world.has_world(); }

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
	uint64_t get_frame() const { return world.get_frame(); }

	void init_snapshots(int slot_count) { world.init_snapshots(slot_count); }
	bool save_state(int slot) { return world.save_state(slot); }
	bool load_state(int slot) { return world.load_state(slot); }
	int64_t state_hash() const { return (int64_t)world.get_state_hash(); }

	int get_body_count() const { return world.get_body_count(); }
	int get_live_body_count() const { return world.get_live_body_count(); }
	int get_awake_body_count() const { return world.get_awake_body_count(); }
	int64_t get_body_id(int handle) const { return world.get_body_id(handle); }
	int64_t get_world_id() const { return world.get_world_id(); }

	void set_player_bodies(int player, const PackedInt64Array &handles);
	PackedInt64Array compute_affected_bodies(int players_mask, int window_frames) const;
	Dictionary get_last_rollback_scope() const;
	PackedFloat32Array get_transforms() const;
	PackedFloat32Array get_body_meta() const;
	Transform3D get_body_transform(int handle) const;

	bool start_recording() { return world.start_recording(); }
	void stop_recording() { world.stop_recording(); }
	bool is_recording() const { return world.is_recording(); }
	bool save_recording(const String &path);
	static bool validate_recording_file(const String &path);

	void set_worker_count(int count) { world.set_worker_count(count); }
	int get_worker_count() const { return world.get_worker_count(); }
	void set_input_count(int count) { world.set_input_count(count); }
	int get_input_count() const { return world.get_input_count(); }
	void set_time_step(float value) { world.set_time_step(value); }
	float get_time_step() const { return world.get_time_step(); }
	void set_sub_steps(int count) { world.set_sub_steps(count); }
	int get_sub_steps() const { return world.get_sub_steps(); }
	double get_last_step_time_ms() const { return world.get_last_step_time_ms(); }

	void rollback_begin(int64_t target_frame, int window_frames, int players_mask) {
		world.rollback_begin(target_frame, window_frames, players_mask);
	}

	// Simulation interface the session's reflection bridge invokes by name.
	bool rollback_has_world() const { return world.has_world(); }
	int rollback_get_input_count() const { return world.get_input_count(); }
	void rollback_init_snapshots(int slot_count) { world.init_snapshots(slot_count); }
	bool rollback_save_state(int slot) { return world.save_state(slot); }
	bool rollback_load_state(int slot) { return world.load_state(slot); }
	int64_t rollback_state_hash() const { return (int64_t)world.get_state_hash(); }
	void rollback_step_frame(const PackedInt64Array &inputs) { step_frame(inputs); }
};

} // namespace godot
