#include "box3d_rollback_world.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include "box3d/collision.h"

#include <chrono>
#include <cmath>
#include <cstring>
#include <unordered_set>

using namespace godot;

namespace {

constexpr uint64_t FNV_INIT = 14695981039346656037ull;
constexpr uint64_t FNV_PRIME = 1099511628211ull;

uint64_t fnv1a(uint64_t h, const void *data, size_t count) {
	const uint8_t *bytes = static_cast<const uint8_t *>(data);
	for (size_t i = 0; i < count; ++i) {
		h ^= bytes[i];
		h *= FNV_PRIME;
	}
	return h;
}

uint64_t fnv_f32(uint64_t h, float value) {
	uint32_t bits;
	std::memcpy(&bits, &value, sizeof(bits));
	return fnv1a(h, &bits, sizeof(bits));
}

b3Vec3 to_b3(const Vector3 &v) {
	return { v.x, v.y, v.z };
}

} // namespace

Box3DRollbackWorld::~Box3DRollbackWorld() {
	destroy_world();
}

void Box3DRollbackWorld::create_world() {
	ERR_FAIL_COND_MSG(has_world(), "Box3DRollbackWorld: world already created.");

	b3WorldDef def = b3DefaultWorldDef();
	def.workerCount = (uint32_t)MAX(1, worker_count);
	world_id = b3CreateWorld(&def);

	frame = 0;
	bodies.clear();
	body_meta.clear();
	side_slots.clear();
	last_step_time_ms = 0.0;
}

void Box3DRollbackWorld::destroy_world() {
	if (has_world()) {
		if (recording_active) {
			b3World_StopRecording(world_id);
			recording_active = false;
		}
		b3DestroyWorld(world_id);
		world_id = b3_nullWorldId;
	}
	if (recording != nullptr) {
		b3DestroyRecording(recording);
		recording = nullptr;
	}
	if (rollback != nullptr) {
		b3r_destroy(rollback);
		rollback = nullptr;
	}
	side_slots.clear();
	bodies.clear();
	body_meta.clear();
	frame = 0;
}

bool Box3DRollbackWorld::has_world() const {
	return b3World_IsValid(world_id);
}

int Box3DRollbackWorld::register_body(b3BodyId id, const Vector3 &meta, BodyKind kind) {
	bodies.push_back(b3StoreBodyId(id));
	body_meta.push_back(meta.x);
	body_meta.push_back(meta.y);
	body_meta.push_back(meta.z);
	body_meta.push_back((float)kind);
	return (int)bodies.size() - 1;
}

int Box3DRollbackWorld::add_static_box(const Vector3 &position, const Vector3 &half_extents, float friction) {
	ERR_FAIL_COND_V(!has_world(), -1);
	b3BodyDef body_def = b3DefaultBodyDef();
	body_def.position = to_b3(position);
	b3BodyId body = b3CreateBody(world_id, &body_def);
	b3ShapeDef shape_def = b3DefaultShapeDef();
	shape_def.baseMaterial.friction = friction;
	b3BoxHull hull = b3MakeBoxHull(half_extents.x, half_extents.y, half_extents.z);
	b3CreateHullShape(body, &shape_def, &hull.base);
	return register_body(body, half_extents, BODY_STATIC);
}

int Box3DRollbackWorld::add_dynamic_box(const Vector3 &position, const Vector3 &half_extents, float density, float friction) {
	ERR_FAIL_COND_V(!has_world(), -1);
	b3BodyDef body_def = b3DefaultBodyDef();
	body_def.type = b3_dynamicBody;
	body_def.position = to_b3(position);
	b3BodyId body = b3CreateBody(world_id, &body_def);
	b3ShapeDef shape_def = b3DefaultShapeDef();
	shape_def.density = density;
	shape_def.baseMaterial.friction = friction;
	b3BoxHull hull = b3MakeBoxHull(half_extents.x, half_extents.y, half_extents.z);
	b3CreateHullShape(body, &shape_def, &hull.base);
	return register_body(body, half_extents, BODY_DYNAMIC);
}

int Box3DRollbackWorld::add_static_sphere(const Vector3 &position, float radius, float friction) {
	ERR_FAIL_COND_V(!has_world(), -1);
	ERR_FAIL_COND_V(radius <= 0.0f, -1);
	b3BodyDef body_def = b3DefaultBodyDef();
	body_def.position = to_b3(position);
	b3BodyId body = b3CreateBody(world_id, &body_def);
	b3ShapeDef shape_def = b3DefaultShapeDef();
	shape_def.baseMaterial.friction = friction;
	b3Sphere sphere = { { 0.0f, 0.0f, 0.0f }, radius };
	b3CreateSphereShape(body, &shape_def, &sphere);
	return register_body(body, Vector3(radius, radius, radius), BODY_STATIC);
}

int Box3DRollbackWorld::add_dynamic_sphere(const Vector3 &position, float radius, float density, float friction) {
	ERR_FAIL_COND_V(!has_world(), -1);
	ERR_FAIL_COND_V(radius <= 0.0f, -1);
	b3BodyDef body_def = b3DefaultBodyDef();
	body_def.type = b3_dynamicBody;
	body_def.position = to_b3(position);
	b3BodyId body = b3CreateBody(world_id, &body_def);
	b3ShapeDef shape_def = b3DefaultShapeDef();
	shape_def.density = density;
	shape_def.baseMaterial.friction = friction;
	b3Sphere sphere = { { 0.0f, 0.0f, 0.0f }, radius };
	b3CreateSphereShape(body, &shape_def, &sphere);
	return register_body(body, Vector3(radius, radius, radius), BODY_DYNAMIC);
}

int Box3DRollbackWorld::add_static_capsule(const Vector3 &position, const Vector3 &point_a, const Vector3 &point_b, float radius, float friction) {
	ERR_FAIL_COND_V(!has_world(), -1);
	ERR_FAIL_COND_V(radius <= 0.0f, -1);
	b3BodyDef body_def = b3DefaultBodyDef();
	body_def.position = to_b3(position);
	b3BodyId body = b3CreateBody(world_id, &body_def);
	b3ShapeDef shape_def = b3DefaultShapeDef();
	shape_def.baseMaterial.friction = friction;
	b3Capsule capsule = { to_b3(point_a), to_b3(point_b), radius };
	b3CreateCapsuleShape(body, &shape_def, &capsule);
	return register_body(body, Vector3(radius, radius, radius), BODY_STATIC);
}

int Box3DRollbackWorld::add_dynamic_capsule(const Vector3 &position, const Vector3 &point_a, const Vector3 &point_b, float radius, float density, float friction) {
	ERR_FAIL_COND_V(!has_world(), -1);
	ERR_FAIL_COND_V(radius <= 0.0f, -1);
	b3BodyDef body_def = b3DefaultBodyDef();
	body_def.type = b3_dynamicBody;
	body_def.position = to_b3(position);
	b3BodyId body = b3CreateBody(world_id, &body_def);
	b3ShapeDef shape_def = b3DefaultShapeDef();
	shape_def.density = density;
	shape_def.baseMaterial.friction = friction;
	b3Capsule capsule = { to_b3(point_a), to_b3(point_b), radius };
	b3CreateCapsuleShape(body, &shape_def, &capsule);
	return register_body(body, Vector3(radius, radius, radius), BODY_DYNAMIC);
}

void Box3DRollbackWorld::set_body_linear_velocity(int handle, const Vector3 &velocity) {
	ERR_FAIL_INDEX(handle, (int)bodies.size());
	ERR_FAIL_COND(bodies[(size_t)handle] == 0);
	b3Body_SetLinearVelocity(b3LoadBodyId(bodies[(size_t)handle]), to_b3(velocity));
}

Vector3 Box3DRollbackWorld::get_body_velocity(int handle) const {
	ERR_FAIL_INDEX_V(handle, (int)bodies.size(), Vector3());
	ERR_FAIL_COND_V(bodies[(size_t)handle] == 0, Vector3());
	const b3Vec3 v = b3Body_GetLinearVelocity(b3LoadBodyId(bodies[(size_t)handle]));
	return Vector3(v.x, v.y, v.z);
}

void Box3DRollbackWorld::apply_body_linear_impulse(int handle, const Vector3 &impulse, bool wake) {
	ERR_FAIL_INDEX(handle, (int)bodies.size());
	ERR_FAIL_COND(bodies[(size_t)handle] == 0);
	b3BodyId body = b3LoadBodyId(bodies[(size_t)handle]);
	const b3WorldTransform xf = b3Body_GetTransform(body);
	b3Body_ApplyLinearImpulse(body, to_b3(impulse), xf.p, wake);
}

void Box3DRollbackWorld::step_frame(const PackedInt64Array &inputs) {
	ERR_FAIL_COND(!has_world());
	ERR_FAIL_COND_MSG(inputs.size() != input_count,
			"Box3DRollbackWorld.step_frame: input count must match get_input_count().");
	const auto t0 = std::chrono::high_resolution_clock::now();
	b3World_Step(world_id, time_step, sub_steps);
	const auto t1 = std::chrono::high_resolution_clock::now();
	last_step_time_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
	frame++;
}

void Box3DRollbackWorld::init_snapshots(int slot_count) {
	ERR_FAIL_COND(!has_world());
	ERR_FAIL_COND(slot_count <= 0);
	if (rollback != nullptr) {
		b3r_destroy(rollback);
	}
	rollback = b3r_create(world_id, slot_count);
	side_slots.assign((size_t)slot_count, SideState());
}

bool Box3DRollbackWorld::save_state(int slot) {
	ERR_FAIL_COND_V_MSG(rollback == nullptr, false, "Box3DRollbackWorld: call init_snapshots() first.");
	ERR_FAIL_INDEX_V(slot, (int)side_slots.size(), false);
	const int bytes = b3r_save(rollback, slot);
	ERR_FAIL_COND_V(bytes <= 0, false);
	SideState &s = side_slots[(size_t)slot];
	s.frame = frame;
	s.bodies = bodies;
	s.body_meta = body_meta;
	s.valid = true;
	return true;
}

bool Box3DRollbackWorld::load_state(int slot) {
	ERR_FAIL_COND_V_MSG(rollback == nullptr, false, "Box3DRollbackWorld: call init_snapshots() first.");
	ERR_FAIL_INDEX_V(slot, (int)side_slots.size(), false);
	const SideState &s = side_slots[(size_t)slot];
	ERR_FAIL_COND_V(!s.valid, false);
	const bool ok = b3r_load(rollback, slot);
	ERR_FAIL_COND_V(!ok, false);
	frame = s.frame;
	bodies = s.bodies;
	body_meta = s.body_meta;
	return true;
}

uint64_t Box3DRollbackWorld::state_hash_u64() const {
	if (!has_world()) {
		return 0;
	}
	uint64_t side = FNV_INIT;
	side = fnv1a(side, &frame, sizeof(frame));
	const uint64_t body_count = (uint64_t)bodies.size();
	side = fnv1a(side, &body_count, sizeof(body_count));
	for (uint64_t stored_body : bodies) {
		const b3BodyId body = b3LoadBodyId(stored_body);
		side = fnv1a(side, &body.index1, sizeof(body.index1));
		side = fnv1a(side, &body.generation, sizeof(body.generation));
	}
	const uint64_t meta_count = (uint64_t)body_meta.size();
	side = fnv1a(side, &meta_count, sizeof(meta_count));
	for (float value : body_meta) {
		side = fnv_f32(side, value);
	}
	return b3r_world_hash(world_id) ^ side;
}

int Box3DRollbackWorld::get_live_body_count() const {
	int count = 0;
	for (uint64_t body : bodies) {
		if (body != 0) {
			count++;
		}
	}
	return count;
}

int Box3DRollbackWorld::get_awake_body_count() const {
	return has_world() ? b3World_GetAwakeBodyCount(world_id) : 0;
}

int64_t Box3DRollbackWorld::get_body_id(int handle) const {
	ERR_FAIL_INDEX_V(handle, (int)bodies.size(), 0);
	return (int64_t)bodies[(size_t)handle];
}

int64_t Box3DRollbackWorld::get_world_id() const {
	return has_world() ? (int64_t)b3StoreWorldId(world_id) : 0;
}

void Box3DRollbackWorld::set_player_bodies(int player, const PackedInt64Array &handles) {
	ERR_FAIL_COND(player < 0 || player >= input_count);
	if ((int)player_body_handles.size() < input_count) {
		player_body_handles.resize((size_t)input_count);
	}
	std::vector<int> &list = player_body_handles[(size_t)player];
	list.clear();
	for (int64_t handle : handles) {
		ERR_FAIL_INDEX((int)handle, (int)bodies.size());
		list.push_back((int)handle);
	}
}

namespace {

struct AffectedCtx {
	std::unordered_set<uint64_t> *visited = nullptr;
	std::vector<b3BodyId> *queue = nullptr;
};

bool traversable(b3BodyId body) {
	return b3Body_IsValid(body) && b3Body_GetType(body) != b3_staticBody;
}

void affected_visit(AffectedCtx &ctx, b3BodyId body) {
	if (!traversable(body)) {
		return;
	}
	const uint64_t key = b3StoreBodyId(body);
	if (ctx.visited->insert(key).second) {
		ctx.queue->push_back(body);
	}
}

bool affected_overlap_cb(b3ShapeId shapeId, void *context) {
	AffectedCtx *ctx = static_cast<AffectedCtx *>(context);
	affected_visit(*ctx, b3Shape_GetBody(shapeId));
	return true;
}

} // namespace

PackedInt64Array Box3DRollbackWorld::compute_affected_bodies(int players_mask, int window_frames) const {
	PackedInt64Array out;
	ERR_FAIL_COND_V(!has_world(), out);
	if (window_frames < 1) {
		window_frames = 1;
	}

	std::unordered_set<uint64_t> visited;
	std::vector<b3BodyId> queue;
	AffectedCtx ctx = { &visited, &queue };

	for (size_t p = 0; p < player_body_handles.size(); ++p) {
		if ((players_mask & (1 << (int)p)) == 0) {
			continue;
		}
		for (int handle : player_body_handles[p]) {
			if (handle >= 0 && handle < (int)bodies.size() && bodies[(size_t)handle] != 0) {
				affected_visit(ctx, b3LoadBodyId(bodies[(size_t)handle]));
			}
		}
	}

	std::vector<b3JointId> joints;
	std::vector<b3ContactData> contacts;
	size_t cursor = 0;
	while (cursor < queue.size()) {
		const b3BodyId body = queue[cursor++];

		const int joint_count = b3Body_GetJointCount(body);
		if (joint_count > 0) {
			joints.resize((size_t)joint_count);
			const int n = b3Body_GetJoints(body, joints.data(), joint_count);
			for (int i = 0; i < n; ++i) {
				affected_visit(ctx, b3Joint_GetBodyA(joints[(size_t)i]));
				affected_visit(ctx, b3Joint_GetBodyB(joints[(size_t)i]));
			}
		}

		const int contact_capacity = b3Body_GetContactCapacity(body);
		if (contact_capacity > 0) {
			contacts.resize((size_t)contact_capacity);
			const int n = b3Body_GetContactData(body, contacts.data(), contact_capacity);
			for (int i = 0; i < n; ++i) {
				affected_visit(ctx, b3Shape_GetBody(contacts[(size_t)i].shapeIdA));
				affected_visit(ctx, b3Shape_GetBody(contacts[(size_t)i].shapeIdB));
			}
		}

		// Anything the body could reach while the window is resimulated joins
		// the closure. The margin is deliberately conservative: straight-line
		// travel at the current speed plus a fixed slop for contact drift.
		b3AABB aabb = b3Body_ComputeAABB(body);
		const b3Vec3 vel = b3Body_GetLinearVelocity(body);
		const float dt = time_step * (float)window_frames;
		const float margin = 0.5f;
		const float ex = std::fabs(vel.x) * dt + margin;
		const float ey = std::fabs(vel.y) * dt + margin;
		const float ez = std::fabs(vel.z) * dt + margin;
		aabb.lowerBound.x -= ex;
		aabb.lowerBound.y -= ey;
		aabb.lowerBound.z -= ez;
		aabb.upperBound.x += ex;
		aabb.upperBound.y += ey;
		aabb.upperBound.z += ez;
		b3World_OverlapAABB(world_id, aabb, b3DefaultQueryFilter(), affected_overlap_cb, &ctx);
	}

	out.resize((int64_t)queue.size());
	for (size_t i = 0; i < queue.size(); ++i) {
		out[(int64_t)i] = (int64_t)b3StoreBodyId(queue[i]);
	}
	out.sort();
	return out;
}

void Box3DRollbackWorld::rollback_begin(int64_t target_frame, int window_frames, int players_mask) {
	if (!has_world()) {
		return;
	}
	last_scope.target_frame = target_frame;
	last_scope.window = MAX(window_frames, 0);
	last_scope.mispredicted_mask = players_mask;
	last_scope.affected_bodies = (int)compute_affected_bodies(players_mask, MAX(last_scope.window, 1)).size();
	last_scope.awake_bodies = get_awake_body_count();
	last_scope.total_bodies = get_live_body_count();
	last_scope.valid = true;
}

Dictionary Box3DRollbackWorld::get_last_rollback_scope() const {
	Dictionary out;
	out["valid"] = last_scope.valid;
	out["target_frame"] = last_scope.target_frame;
	out["window"] = last_scope.window;
	out["mispredicted_mask"] = last_scope.mispredicted_mask;
	out["affected_bodies"] = last_scope.affected_bodies;
	out["awake_bodies"] = last_scope.awake_bodies;
	out["total_bodies"] = last_scope.total_bodies;
	return out;
}

PackedFloat32Array Box3DRollbackWorld::get_transforms() const {
	PackedFloat32Array out;
	out.resize((int64_t)bodies.size() * 7);
	float *w = out.ptrw();
	for (size_t i = 0; i < bodies.size(); ++i) {
		if (bodies[i] == 0) {
			w[7 * i + 6] = 1.0f;
			continue;
		}
		const b3WorldTransform xf = b3Body_GetTransform(b3LoadBodyId(bodies[i]));
		w[7 * i + 0] = xf.p.x;
		w[7 * i + 1] = xf.p.y;
		w[7 * i + 2] = xf.p.z;
		w[7 * i + 3] = xf.q.v.x;
		w[7 * i + 4] = xf.q.v.y;
		w[7 * i + 5] = xf.q.v.z;
		w[7 * i + 6] = xf.q.s;
	}
	return out;
}

PackedFloat32Array Box3DRollbackWorld::get_body_meta() const {
	PackedFloat32Array out;
	out.resize((int64_t)body_meta.size());
	if (!body_meta.empty()) {
		std::memcpy(out.ptrw(), body_meta.data(), body_meta.size() * sizeof(float));
	}
	return out;
}

Transform3D Box3DRollbackWorld::get_body_transform(int handle) const {
	ERR_FAIL_INDEX_V(handle, (int)bodies.size(), Transform3D());
	ERR_FAIL_COND_V(bodies[(size_t)handle] == 0, Transform3D());
	const b3WorldTransform xf = b3Body_GetTransform(b3LoadBodyId(bodies[(size_t)handle]));
	return Transform3D(Quaternion(xf.q.v.x, xf.q.v.y, xf.q.v.z, xf.q.s), Vector3(xf.p.x, xf.p.y, xf.p.z));
}

bool Box3DRollbackWorld::start_recording() {
	ERR_FAIL_COND_V(!has_world(), false);
	ERR_FAIL_COND_V_MSG(recording_active, false, "Box3DRollbackWorld: already recording.");
	if (recording == nullptr) {
		recording = b3CreateRecording(0);
	}
	b3World_StartRecording(world_id, recording);
	recording_active = true;
	return true;
}

void Box3DRollbackWorld::stop_recording() {
	if (!recording_active || !has_world()) {
		return;
	}
	b3World_StopRecording(world_id);
	recording_active = false;
}

bool Box3DRollbackWorld::save_recording(const String &path) {
	ERR_FAIL_NULL_V(recording, false);
	if (recording_active) {
		stop_recording();
	}
	return b3SaveRecordingToFile(recording, path.utf8().get_data());
}

bool Box3DRollbackWorld::validate_recording_file(const String &path) {
	b3Recording *rec = b3LoadRecordingFromFile(path.utf8().get_data());
	if (rec == nullptr) {
		return false;
	}
	const bool ok = b3ValidateReplay(b3Recording_GetData(rec), b3Recording_GetSize(rec), 1);
	b3DestroyRecording(rec);
	return ok;
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
