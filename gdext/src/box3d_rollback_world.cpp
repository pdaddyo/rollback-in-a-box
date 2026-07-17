#include "box3d_rollback_world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include <cstring>

using namespace godot;

namespace {

rb::Vec3 to_rb(const Vector3 &v) {
	return rb::Vec3{ (float)v.x, (float)v.y, (float)v.z };
}

Vector3 to_godot(const rb::Vec3 &v) {
	return Vector3(v.x, v.y, v.z);
}

} // namespace

int Box3DRollbackWorld::add_static_box(const Vector3 &position, const Vector3 &half_extents, float friction) {
	return world.add_static_box(to_rb(position), to_rb(half_extents), friction);
}

int Box3DRollbackWorld::add_dynamic_box(const Vector3 &position, const Vector3 &half_extents, float density, float friction) {
	return world.add_dynamic_box(to_rb(position), to_rb(half_extents), density, friction);
}

int Box3DRollbackWorld::add_static_sphere(const Vector3 &position, float radius, float friction) {
	return world.add_static_sphere(to_rb(position), radius, friction);
}

int Box3DRollbackWorld::add_dynamic_sphere(const Vector3 &position, float radius, float density, float friction) {
	return world.add_dynamic_sphere(to_rb(position), radius, density, friction);
}

int Box3DRollbackWorld::add_static_capsule(const Vector3 &position, const Vector3 &point_a, const Vector3 &point_b, float radius, float friction) {
	return world.add_static_capsule(to_rb(position), to_rb(point_a), to_rb(point_b), radius, friction);
}

int Box3DRollbackWorld::add_dynamic_capsule(const Vector3 &position, const Vector3 &point_a, const Vector3 &point_b, float radius, float density, float friction) {
	return world.add_dynamic_capsule(to_rb(position), to_rb(point_a), to_rb(point_b), radius, density, friction);
}

void Box3DRollbackWorld::set_body_linear_velocity(int handle, const Vector3 &velocity) {
	world.set_body_linear_velocity(handle, to_rb(velocity));
}

Vector3 Box3DRollbackWorld::get_body_velocity(int handle) const {
	return to_godot(world.get_body_velocity(handle));
}

void Box3DRollbackWorld::apply_body_linear_impulse(int handle, const Vector3 &impulse, bool wake) {
	world.apply_body_linear_impulse(handle, to_rb(impulse), wake);
}

void Box3DRollbackWorld::step_frame(const PackedInt64Array &inputs) {
	world.step_frame(inputs.ptr(), (int)inputs.size());
}

void Box3DRollbackWorld::set_player_bodies(int player, const PackedInt64Array &handles) {
	world.set_player_bodies(player, handles.ptr(), (int)handles.size());
}

PackedInt64Array Box3DRollbackWorld::compute_affected_bodies(int players_mask, int window_frames) const {
	const std::vector<int64_t> affected = world.compute_affected_bodies(players_mask, window_frames);
	PackedInt64Array out;
	out.resize((int64_t)affected.size());
	for (size_t i = 0; i < affected.size(); ++i) {
		out[(int64_t)i] = affected[i];
	}
	return out;
}

Dictionary Box3DRollbackWorld::get_last_rollback_scope() const {
	const rb::RollbackScope &scope = world.get_last_rollback_scope();
	Dictionary out;
	out["valid"] = scope.valid;
	out["target_frame"] = scope.target_frame;
	out["window"] = scope.window;
	out["mispredicted_mask"] = scope.mispredicted_mask;
	out["affected_bodies"] = scope.affected_bodies;
	out["awake_bodies"] = scope.awake_bodies;
	out["total_bodies"] = scope.total_bodies;
	return out;
}

PackedFloat32Array Box3DRollbackWorld::get_transforms() const {
	const std::vector<float> transforms = world.get_transforms();
	PackedFloat32Array out;
	out.resize((int64_t)transforms.size());
	if (!transforms.empty()) {
		std::memcpy(out.ptrw(), transforms.data(), transforms.size() * sizeof(float));
	}
	return out;
}

PackedFloat32Array Box3DRollbackWorld::get_body_meta() const {
	const std::vector<float> &meta = world.get_body_meta();
	PackedFloat32Array out;
	out.resize((int64_t)meta.size());
	if (!meta.empty()) {
		std::memcpy(out.ptrw(), meta.data(), meta.size() * sizeof(float));
	}
	return out;
}

Transform3D Box3DRollbackWorld::get_body_transform(int handle) const {
	const rb::Transform xf = world.get_body_transform(handle);
	return Transform3D(Quaternion(xf.q.x, xf.q.y, xf.q.z, xf.q.w), Vector3(xf.p.x, xf.p.y, xf.p.z));
}

bool Box3DRollbackWorld::save_recording(const String &path) {
	return world.save_recording(path.utf8().get_data());
}

bool Box3DRollbackWorld::validate_recording_file(const String &path) {
	return rb::RollbackWorld::validate_recording_file(path.utf8().get_data());
}

void Box3DRollbackWorld::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_world"), &Box3DRollbackWorld::create_world);
	ClassDB::bind_method(D_METHOD("destroy_world"), &Box3DRollbackWorld::destroy_world);
	ClassDB::bind_method(D_METHOD("has_world"), &Box3DRollbackWorld::has_world);
	ClassDB::bind_method(D_METHOD("add_static_box", "position", "half_extents", "friction"), &Box3DRollbackWorld::add_static_box, DEFVAL(0.6f));
	ClassDB::bind_method(D_METHOD("add_dynamic_box", "position", "half_extents", "density", "friction"), &Box3DRollbackWorld::add_dynamic_box, DEFVAL(300.0f), DEFVAL(0.6f));
	ClassDB::bind_method(D_METHOD("add_static_sphere", "position", "radius", "friction"), &Box3DRollbackWorld::add_static_sphere, DEFVAL(0.6f));
	ClassDB::bind_method(D_METHOD("add_dynamic_sphere", "position", "radius", "density", "friction"), &Box3DRollbackWorld::add_dynamic_sphere, DEFVAL(300.0f), DEFVAL(0.6f));
	ClassDB::bind_method(D_METHOD("add_static_capsule", "position", "point_a", "point_b", "radius", "friction"), &Box3DRollbackWorld::add_static_capsule, DEFVAL(0.6f));
	ClassDB::bind_method(D_METHOD("add_dynamic_capsule", "position", "point_a", "point_b", "radius", "density", "friction"), &Box3DRollbackWorld::add_dynamic_capsule, DEFVAL(300.0f), DEFVAL(0.6f));
	ClassDB::bind_method(D_METHOD("set_body_linear_velocity", "handle", "velocity"), &Box3DRollbackWorld::set_body_linear_velocity);
	ClassDB::bind_method(D_METHOD("get_body_velocity", "handle"), &Box3DRollbackWorld::get_body_velocity);
	ClassDB::bind_method(D_METHOD("apply_body_linear_impulse", "handle", "impulse", "wake"), &Box3DRollbackWorld::apply_body_linear_impulse, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("step_frame", "inputs"), &Box3DRollbackWorld::step_frame);
	ClassDB::bind_method(D_METHOD("get_frame"), &Box3DRollbackWorld::get_frame);
	ClassDB::bind_method(D_METHOD("init_snapshots", "slot_count"), &Box3DRollbackWorld::init_snapshots);
	ClassDB::bind_method(D_METHOD("save_state", "slot"), &Box3DRollbackWorld::save_state);
	ClassDB::bind_method(D_METHOD("load_state", "slot"), &Box3DRollbackWorld::load_state);
	ClassDB::bind_method(D_METHOD("state_hash"), &Box3DRollbackWorld::state_hash);
	ClassDB::bind_method(D_METHOD("get_body_count"), &Box3DRollbackWorld::get_body_count);
	ClassDB::bind_method(D_METHOD("get_live_body_count"), &Box3DRollbackWorld::get_live_body_count);
	ClassDB::bind_method(D_METHOD("get_awake_body_count"), &Box3DRollbackWorld::get_awake_body_count);
	ClassDB::bind_method(D_METHOD("get_body_id", "handle"), &Box3DRollbackWorld::get_body_id);
	ClassDB::bind_method(D_METHOD("get_world_id"), &Box3DRollbackWorld::get_world_id);
	ClassDB::bind_method(D_METHOD("set_player_bodies", "player", "handles"), &Box3DRollbackWorld::set_player_bodies);
	ClassDB::bind_method(D_METHOD("compute_affected_bodies", "players_mask", "window_frames"), &Box3DRollbackWorld::compute_affected_bodies);
	ClassDB::bind_method(D_METHOD("get_last_rollback_scope"), &Box3DRollbackWorld::get_last_rollback_scope);
	ClassDB::bind_method(D_METHOD("rollback_begin", "target_frame", "window_frames", "players_mask"), &Box3DRollbackWorld::rollback_begin);
	ClassDB::bind_method(D_METHOD("get_transforms"), &Box3DRollbackWorld::get_transforms);
	ClassDB::bind_method(D_METHOD("get_body_meta"), &Box3DRollbackWorld::get_body_meta);
	ClassDB::bind_method(D_METHOD("get_body_transform", "handle"), &Box3DRollbackWorld::get_body_transform);
	ClassDB::bind_method(D_METHOD("start_recording"), &Box3DRollbackWorld::start_recording);
	ClassDB::bind_method(D_METHOD("stop_recording"), &Box3DRollbackWorld::stop_recording);
	ClassDB::bind_method(D_METHOD("is_recording"), &Box3DRollbackWorld::is_recording);
	ClassDB::bind_method(D_METHOD("save_recording", "path"), &Box3DRollbackWorld::save_recording);
	ClassDB::bind_static_method("Box3DRollbackWorld", D_METHOD("validate_recording_file", "path"), &Box3DRollbackWorld::validate_recording_file);
	ClassDB::bind_method(D_METHOD("set_worker_count", "count"), &Box3DRollbackWorld::set_worker_count);
	ClassDB::bind_method(D_METHOD("get_worker_count"), &Box3DRollbackWorld::get_worker_count);
	ClassDB::bind_method(D_METHOD("set_input_count", "count"), &Box3DRollbackWorld::set_input_count);
	ClassDB::bind_method(D_METHOD("get_input_count"), &Box3DRollbackWorld::get_input_count);
	ClassDB::bind_method(D_METHOD("set_time_step", "value"), &Box3DRollbackWorld::set_time_step);
	ClassDB::bind_method(D_METHOD("get_time_step"), &Box3DRollbackWorld::get_time_step);
	ClassDB::bind_method(D_METHOD("set_sub_steps", "count"), &Box3DRollbackWorld::set_sub_steps);
	ClassDB::bind_method(D_METHOD("get_sub_steps"), &Box3DRollbackWorld::get_sub_steps);
	ClassDB::bind_method(D_METHOD("get_last_step_time_ms"), &Box3DRollbackWorld::get_last_step_time_ms);
	ClassDB::bind_method(D_METHOD("rollback_has_world"), &Box3DRollbackWorld::rollback_has_world);
	ClassDB::bind_method(D_METHOD("rollback_get_input_count"), &Box3DRollbackWorld::rollback_get_input_count);
	ClassDB::bind_method(D_METHOD("rollback_init_snapshots", "slot_count"), &Box3DRollbackWorld::rollback_init_snapshots);
	ClassDB::bind_method(D_METHOD("rollback_save_state", "slot"), &Box3DRollbackWorld::rollback_save_state);
	ClassDB::bind_method(D_METHOD("rollback_load_state", "slot"), &Box3DRollbackWorld::rollback_load_state);
	ClassDB::bind_method(D_METHOD("rollback_state_hash"), &Box3DRollbackWorld::rollback_state_hash);
	ClassDB::bind_method(D_METHOD("rollback_step_frame", "inputs"), &Box3DRollbackWorld::rollback_step_frame);
	BIND_CONSTANT(BODY_NONE);
	BIND_CONSTANT(BODY_STATIC);
	BIND_CONSTANT(BODY_DYNAMIC);
}
