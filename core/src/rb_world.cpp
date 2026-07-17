#include "box3d_rollback/rb_world.h"

#include "box3d/collision.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <unordered_set>

namespace rb {

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

b3Vec3 to_b3(const Vec3 &v) {
	return { v.x, v.y, v.z };
}

} // namespace

RollbackWorld::~RollbackWorld() {
	destroy_world();
}

void RollbackWorld::create_world() {
	if (has_world()) {
		return;
	}

	b3WorldDef def = b3DefaultWorldDef();
	def.workerCount = (uint32_t)(worker_count < 1 ? 1 : worker_count);
	world_id = b3CreateWorld(&def);

	frame = 0;
	bodies.clear();
	body_meta.clear();
	side_slots.clear();
	last_step_time_ms = 0.0;
}

void RollbackWorld::destroy_world() {
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

bool RollbackWorld::has_world() const {
	return b3World_IsValid(world_id);
}

int RollbackWorld::register_body(b3BodyId id, const Vec3 &meta, BodyKind kind) {
	bodies.push_back(b3StoreBodyId(id));
	body_meta.push_back(meta.x);
	body_meta.push_back(meta.y);
	body_meta.push_back(meta.z);
	body_meta.push_back((float)kind);
	return (int)bodies.size() - 1;
}

int RollbackWorld::add_static_box(const Vec3 &position, const Vec3 &half_extents, float friction) {
	if (!has_world()) {
		return -1;
	}
	b3BodyDef body_def = b3DefaultBodyDef();
	body_def.position = to_b3(position);
	b3BodyId body = b3CreateBody(world_id, &body_def);
	b3ShapeDef shape_def = b3DefaultShapeDef();
	shape_def.baseMaterial.friction = friction;
	b3BoxHull hull = b3MakeBoxHull(half_extents.x, half_extents.y, half_extents.z);
	b3CreateHullShape(body, &shape_def, &hull.base);
	return register_body(body, half_extents, BODY_STATIC);
}

int RollbackWorld::add_dynamic_box(const Vec3 &position, const Vec3 &half_extents, float density, float friction) {
	if (!has_world()) {
		return -1;
	}
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

int RollbackWorld::add_static_sphere(const Vec3 &position, float radius, float friction) {
	if (!has_world()) {
		return -1;
	}
	if (radius <= 0.0f) {
		return -1;
	}
	b3BodyDef body_def = b3DefaultBodyDef();
	body_def.position = to_b3(position);
	b3BodyId body = b3CreateBody(world_id, &body_def);
	b3ShapeDef shape_def = b3DefaultShapeDef();
	shape_def.baseMaterial.friction = friction;
	b3Sphere sphere = { { 0.0f, 0.0f, 0.0f }, radius };
	b3CreateSphereShape(body, &shape_def, &sphere);
	return register_body(body, Vec3{ radius, radius, radius }, BODY_STATIC);
}

int RollbackWorld::add_dynamic_sphere(const Vec3 &position, float radius, float density, float friction) {
	if (!has_world()) {
		return -1;
	}
	if (radius <= 0.0f) {
		return -1;
	}
	b3BodyDef body_def = b3DefaultBodyDef();
	body_def.type = b3_dynamicBody;
	body_def.position = to_b3(position);
	b3BodyId body = b3CreateBody(world_id, &body_def);
	b3ShapeDef shape_def = b3DefaultShapeDef();
	shape_def.density = density;
	shape_def.baseMaterial.friction = friction;
	b3Sphere sphere = { { 0.0f, 0.0f, 0.0f }, radius };
	b3CreateSphereShape(body, &shape_def, &sphere);
	return register_body(body, Vec3{ radius, radius, radius }, BODY_DYNAMIC);
}

int RollbackWorld::add_static_capsule(const Vec3 &position, const Vec3 &point_a, const Vec3 &point_b, float radius, float friction) {
	if (!has_world()) {
		return -1;
	}
	if (radius <= 0.0f) {
		return -1;
	}
	b3BodyDef body_def = b3DefaultBodyDef();
	body_def.position = to_b3(position);
	b3BodyId body = b3CreateBody(world_id, &body_def);
	b3ShapeDef shape_def = b3DefaultShapeDef();
	shape_def.baseMaterial.friction = friction;
	b3Capsule capsule = { to_b3(point_a), to_b3(point_b), radius };
	b3CreateCapsuleShape(body, &shape_def, &capsule);
	return register_body(body, Vec3{ radius, radius, radius }, BODY_STATIC);
}

int RollbackWorld::add_dynamic_capsule(const Vec3 &position, const Vec3 &point_a, const Vec3 &point_b, float radius, float density, float friction) {
	if (!has_world()) {
		return -1;
	}
	if (radius <= 0.0f) {
		return -1;
	}
	b3BodyDef body_def = b3DefaultBodyDef();
	body_def.type = b3_dynamicBody;
	body_def.position = to_b3(position);
	b3BodyId body = b3CreateBody(world_id, &body_def);
	b3ShapeDef shape_def = b3DefaultShapeDef();
	shape_def.density = density;
	shape_def.baseMaterial.friction = friction;
	b3Capsule capsule = { to_b3(point_a), to_b3(point_b), radius };
	b3CreateCapsuleShape(body, &shape_def, &capsule);
	return register_body(body, Vec3{ radius, radius, radius }, BODY_DYNAMIC);
}

void RollbackWorld::set_body_linear_velocity(int handle, const Vec3 &velocity) {
	if (handle < 0 || handle >= (int)bodies.size()) {
		return;
	}
	if (bodies[(size_t)handle] == 0) {
		return;
	}
	b3Body_SetLinearVelocity(b3LoadBodyId(bodies[(size_t)handle]), to_b3(velocity));
}

Vec3 RollbackWorld::get_body_velocity(int handle) const {
	if (handle < 0 || handle >= (int)bodies.size()) {
		return Vec3();
	}
	if (bodies[(size_t)handle] == 0) {
		return Vec3();
	}
	const b3Vec3 v = b3Body_GetLinearVelocity(b3LoadBodyId(bodies[(size_t)handle]));
	return Vec3{ v.x, v.y, v.z };
}

void RollbackWorld::apply_body_linear_impulse(int handle, const Vec3 &impulse, bool wake) {
	if (handle < 0 || handle >= (int)bodies.size()) {
		return;
	}
	if (bodies[(size_t)handle] == 0) {
		return;
	}
	b3BodyId body = b3LoadBodyId(bodies[(size_t)handle]);
	const b3WorldTransform xf = b3Body_GetTransform(body);
	b3Body_ApplyLinearImpulse(body, to_b3(impulse), xf.p, wake);
}

void RollbackWorld::step_frame(const int64_t *inputs, int count) {
	(void)inputs;
	if (!has_world()) {
		return;
	}
	if (count != input_count) {
		return;
	}
	const auto t0 = std::chrono::high_resolution_clock::now();
	b3World_Step(world_id, time_step, sub_steps);
	const auto t1 = std::chrono::high_resolution_clock::now();
	last_step_time_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
	frame++;
}

void RollbackWorld::init_snapshots(int slot_count) {
	if (!has_world()) {
		return;
	}
	if (slot_count <= 0) {
		return;
	}
	if (rollback != nullptr) {
		b3r_destroy(rollback);
	}
	rollback = b3r_create(world_id, slot_count);
	side_slots.assign((size_t)slot_count, SideState());
}

bool RollbackWorld::save_state(int slot) {
	if (rollback == nullptr) {
		return false;
	}
	if (slot < 0 || slot >= (int)side_slots.size()) {
		return false;
	}
	const int bytes = b3r_save(rollback, slot);
	if (bytes <= 0) {
		return false;
	}
	SideState &s = side_slots[(size_t)slot];
	s.frame = frame;
	s.bodies = bodies;
	s.body_meta = body_meta;
	s.valid = true;
	return true;
}

bool RollbackWorld::load_state(int slot) {
	if (rollback == nullptr) {
		return false;
	}
	if (slot < 0 || slot >= (int)side_slots.size()) {
		return false;
	}
	const SideState &s = side_slots[(size_t)slot];
	if (!s.valid) {
		return false;
	}
	const bool ok = b3r_load(rollback, slot);
	if (!ok) {
		return false;
	}
	frame = s.frame;
	bodies = s.bodies;
	body_meta = s.body_meta;
	return true;
}

uint64_t RollbackWorld::state_hash_u64() const {
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

int RollbackWorld::get_live_body_count() const {
	int count = 0;
	for (uint64_t body : bodies) {
		if (body != 0) {
			count++;
		}
	}
	return count;
}

int RollbackWorld::get_awake_body_count() const {
	return has_world() ? b3World_GetAwakeBodyCount(world_id) : 0;
}

int64_t RollbackWorld::get_body_id(int handle) const {
	if (handle < 0 || handle >= (int)bodies.size()) {
		return 0;
	}
	return (int64_t)bodies[(size_t)handle];
}

int64_t RollbackWorld::get_world_id() const {
	return has_world() ? (int64_t)b3StoreWorldId(world_id) : 0;
}

void RollbackWorld::set_player_bodies(int player, const int64_t *handles, int count) {
	if (player < 0 || player >= input_count) {
		return;
	}
	if ((int)player_body_handles.size() < input_count) {
		player_body_handles.resize((size_t)input_count);
	}
	std::vector<int> &list = player_body_handles[(size_t)player];
	list.clear();
	for (int i = 0; i < count; ++i) {
		const int64_t handle = handles[i];
		if (handle < 0 || handle >= (int64_t)bodies.size()) {
			return;
		}
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

std::vector<int64_t> RollbackWorld::compute_affected_bodies(int players_mask, int window_frames) const {
	std::vector<int64_t> out;
	if (!has_world()) {
		return out;
	}
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

	out.resize(queue.size());
	for (size_t i = 0; i < queue.size(); ++i) {
		out[i] = (int64_t)b3StoreBodyId(queue[i]);
	}
	std::sort(out.begin(), out.end());
	return out;
}

void RollbackWorld::rollback_begin(int64_t target_frame, int window_frames, int players_mask) {
	if (!has_world()) {
		return;
	}
	last_scope.target_frame = target_frame;
	last_scope.window = window_frames > 0 ? window_frames : 0;
	last_scope.mispredicted_mask = players_mask;
	last_scope.affected_bodies = (int)compute_affected_bodies(players_mask, last_scope.window > 1 ? last_scope.window : 1).size();
	last_scope.awake_bodies = get_awake_body_count();
	last_scope.total_bodies = get_live_body_count();
	last_scope.valid = true;
}

std::vector<float> RollbackWorld::get_transforms() const {
	std::vector<float> out;
	out.resize(bodies.size() * 7);
	float *w = out.data();
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

Transform RollbackWorld::get_body_transform(int handle) const {
	if (handle < 0 || handle >= (int)bodies.size()) {
		return Transform();
	}
	if (bodies[(size_t)handle] == 0) {
		return Transform();
	}
	const b3WorldTransform xf = b3Body_GetTransform(b3LoadBodyId(bodies[(size_t)handle]));
	Transform out;
	out.p = Vec3{ xf.p.x, xf.p.y, xf.p.z };
	out.q = Quat{ xf.q.v.x, xf.q.v.y, xf.q.v.z, xf.q.s };
	return out;
}

bool RollbackWorld::start_recording() {
	if (!has_world()) {
		return false;
	}
	if (recording_active) {
		return false;
	}
	if (recording == nullptr) {
		recording = b3CreateRecording(0);
	}
	b3World_StartRecording(world_id, recording);
	recording_active = true;
	return true;
}

void RollbackWorld::stop_recording() {
	if (!recording_active || !has_world()) {
		return;
	}
	b3World_StopRecording(world_id);
	recording_active = false;
}

bool RollbackWorld::save_recording(const char *path) {
	if (recording == nullptr) {
		return false;
	}
	if (recording_active) {
		stop_recording();
	}
	return b3SaveRecordingToFile(recording, path);
}

bool RollbackWorld::validate_recording_file(const char *path) {
	b3Recording *rec = b3LoadRecordingFromFile(path);
	if (rec == nullptr) {
		return false;
	}
	const bool ok = b3ValidateReplay(b3Recording_GetData(rec), b3Recording_GetSize(rec), 1);
	b3DestroyRecording(rec);
	return ok;
}

} // namespace rb
