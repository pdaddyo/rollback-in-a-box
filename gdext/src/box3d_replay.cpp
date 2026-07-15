#include "box3d_replay.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>

#include <algorithm>
#include <cstring>

namespace godot {

namespace {

bool collect_shape_cb(b3ShapeId shape, void *context) {
	static_cast<std::vector<b3ShapeId> *>(context)->push_back(shape);
	return true;
}

int64_t pack_shape_id(b3ShapeId id) {
	return ((int64_t)id.index1 << 32) | (int64_t)id.generation;
}

} // namespace

Box3DReplay::~Box3DReplay() {
	close();
}

void Box3DReplay::close() {
	if (player != nullptr) {
		b3RecPlayer_Destroy(player);
		player = nullptr;
	}
}

bool Box3DReplay::load_file(const String &path) {
	close();
	b3Recording *rec = b3LoadRecordingFromFile(path.utf8().get_data());
	if (rec == nullptr) {
		return false;
	}
	// The player keeps a private copy of the bytes, so the recording can go.
	player = b3RecPlayer_Create(b3Recording_GetData(rec), b3Recording_GetSize(rec), 1);
	b3DestroyRecording(rec);
	return player != nullptr;
}

Dictionary Box3DReplay::get_info() const {
	Dictionary out;
	ERR_FAIL_NULL_V(player, out);
	const b3RecPlayerInfo info = b3RecPlayer_GetInfo(player);
	out["frame_count"] = info.frameCount;
	out["time_step"] = info.timeStep;
	out["sub_steps"] = info.subStepCount;
	out["bounds_min"] = Vector3(info.bounds.lowerBound.x, info.bounds.lowerBound.y, info.bounds.lowerBound.z);
	out["bounds_max"] = Vector3(info.bounds.upperBound.x, info.bounds.upperBound.y, info.bounds.upperBound.z);
	return out;
}

bool Box3DReplay::step_frame() {
	ERR_FAIL_NULL_V(player, false);
	return b3RecPlayer_StepFrame(player);
}

void Box3DReplay::seek_frame(int frame) {
	ERR_FAIL_NULL(player);
	b3RecPlayer_SeekFrame(player, frame);
}

void Box3DReplay::restart() {
	ERR_FAIL_NULL(player);
	b3RecPlayer_Restart(player);
}

int Box3DReplay::get_frame() const {
	return player != nullptr ? b3RecPlayer_GetFrame(player) : 0;
}

int Box3DReplay::get_frame_count() const {
	return player != nullptr ? b3RecPlayer_GetFrameCount(player) : 0;
}

bool Box3DReplay::is_at_end() const {
	return player == nullptr || b3RecPlayer_IsAtEnd(player);
}

bool Box3DReplay::has_diverged() const {
	return player != nullptr && b3RecPlayer_HasDiverged(player);
}

int Box3DReplay::get_diverge_frame() const {
	return player != nullptr ? b3RecPlayer_GetDivergeFrame(player) : -1;
}

std::vector<b3ShapeId> Box3DReplay::collect_shapes() const {
	std::vector<b3ShapeId> shapes;
	if (player == nullptr) {
		return shapes;
	}
	const b3WorldId world = b3RecPlayer_GetWorldId(player);
	if (!b3World_IsValid(world)) {
		return shapes;
	}
	// Query over the recording's accumulated bounds (padded); this is the
	// documented way to enumerate shapes without touching world internals.
	const b3RecPlayerInfo info = b3RecPlayer_GetInfo(player);
	b3AABB aabb = info.bounds;
	const bool degenerate = aabb.upperBound.x - aabb.lowerBound.x <= 0.0f;
	if (degenerate) {
		aabb.lowerBound = { -10000.0f, -10000.0f, -10000.0f };
		aabb.upperBound = { 10000.0f, 10000.0f, 10000.0f };
	} else {
		const float pad = 100.0f;
		aabb.lowerBound.x -= pad; aabb.lowerBound.y -= pad; aabb.lowerBound.z -= pad;
		aabb.upperBound.x += pad; aabb.upperBound.y += pad; aabb.upperBound.z += pad;
	}
	b3QueryFilter filter = b3DefaultQueryFilter();
	filter.maskBits = ~(uint64_t)0;
	b3World_OverlapAABB(world, aabb, filter, collect_shape_cb, &shapes);
	// Tree traversal order is an implementation detail — sort for stability so
	// get_shape_ids and get_shape_transforms always line up.
	std::sort(shapes.begin(), shapes.end(), [](const b3ShapeId &a, const b3ShapeId &b) {
		return a.index1 != b.index1 ? a.index1 < b.index1 : a.generation < b.generation;
	});
	return shapes;
}

PackedInt64Array Box3DReplay::get_shape_ids() const {
	PackedInt64Array out;
	for (const b3ShapeId &id : collect_shapes()) {
		out.push_back(pack_shape_id(id));
	}
	return out;
}

Dictionary Box3DReplay::get_shape_geometry(int64_t packed_id) const {
	Dictionary out;
	ERR_FAIL_NULL_V(player, out);
	// Resolve the packed id against the live enumeration rather than
	// reconstructing a b3ShapeId by hand (the world0 field is an internal
	// index we should not guess at).
	b3ShapeId id = b3_nullShapeId;
	for (const b3ShapeId &sid : collect_shapes()) {
		if (pack_shape_id(sid) == packed_id) {
			id = sid;
			break;
		}
	}
	if (!b3Shape_IsValid(id)) {
		return out;
	}
	out["dynamic"] = b3Body_GetType(b3Shape_GetBody(id)) == b3_dynamicBody;
	switch (b3Shape_GetType(id)) {
		case b3_sphereShape: {
			const b3Sphere s = b3Shape_GetSphere(id);
			out["type"] = "sphere";
			out["center"] = Vector3(s.center.x, s.center.y, s.center.z);
			out["radius"] = s.radius;
		} break;
		case b3_capsuleShape: {
			const b3Capsule c = b3Shape_GetCapsule(id);
			out["type"] = "capsule";
			out["a"] = Vector3(c.center1.x, c.center1.y, c.center1.z);
			out["b"] = Vector3(c.center2.x, c.center2.y, c.center2.z);
			out["radius"] = c.radius;
		} break;
		case b3_hullShape: {
			const b3HullData *hull = b3Shape_GetHull(id);
			out["type"] = "hull";
			PackedVector3Array points;
			if (hull != nullptr) {
				const b3Vec3 *pts = (const b3Vec3 *)((const char *)hull + hull->pointOffset);
				for (int i = 0; i < hull->vertexCount; ++i) {
					points.push_back(Vector3(pts[i].x, pts[i].y, pts[i].z));
				}
			}
			out["points"] = points;
		} break;
		default: {
			// Meshes / heightfields / compounds: give the viewer a local-space
			// box (world AABB pulled back through the body transform would be
			// overkill for terrain — static shapes render fine from the world
			// AABB since their transform never changes).
			const b3AABB aabb = b3Shape_GetAABB(id);
			out["type"] = "bounds";
			out["min"] = Vector3(aabb.lowerBound.x, aabb.lowerBound.y, aabb.lowerBound.z);
			out["max"] = Vector3(aabb.upperBound.x, aabb.upperBound.y, aabb.upperBound.z);
		} break;
	}
	return out;
}

PackedFloat32Array Box3DReplay::get_shape_transforms() const {
	PackedFloat32Array out;
	const std::vector<b3ShapeId> shapes = collect_shapes();
	out.resize((int64_t)shapes.size() * 7);
	float *w = out.ptrw();
	for (size_t i = 0; i < shapes.size(); ++i) {
		const b3BodyId body = b3Shape_GetBody(shapes[i]);
		const b3Pos p = b3Body_GetPosition(body);
		const b3Quat q = b3Body_GetRotation(body);
		w[i * 7 + 0] = (float)p.x;
		w[i * 7 + 1] = (float)p.y;
		w[i * 7 + 2] = (float)p.z;
		w[i * 7 + 3] = q.v.x;
		w[i * 7 + 4] = q.v.y;
		w[i * 7 + 5] = q.v.z;
		w[i * 7 + 6] = q.s;
	}
	return out;
}

void Box3DReplay::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_file", "path"), &Box3DReplay::load_file);
	ClassDB::bind_method(D_METHOD("close"), &Box3DReplay::close);
	ClassDB::bind_method(D_METHOD("is_loaded"), &Box3DReplay::is_loaded);
	ClassDB::bind_method(D_METHOD("get_info"), &Box3DReplay::get_info);
	ClassDB::bind_method(D_METHOD("step_frame"), &Box3DReplay::step_frame);
	ClassDB::bind_method(D_METHOD("seek_frame", "frame"), &Box3DReplay::seek_frame);
	ClassDB::bind_method(D_METHOD("restart"), &Box3DReplay::restart);
	ClassDB::bind_method(D_METHOD("get_frame"), &Box3DReplay::get_frame);
	ClassDB::bind_method(D_METHOD("get_frame_count"), &Box3DReplay::get_frame_count);
	ClassDB::bind_method(D_METHOD("is_at_end"), &Box3DReplay::is_at_end);
	ClassDB::bind_method(D_METHOD("has_diverged"), &Box3DReplay::has_diverged);
	ClassDB::bind_method(D_METHOD("get_diverge_frame"), &Box3DReplay::get_diverge_frame);
	ClassDB::bind_method(D_METHOD("get_shape_ids"), &Box3DReplay::get_shape_ids);
	ClassDB::bind_method(D_METHOD("get_shape_geometry", "packed_id"), &Box3DReplay::get_shape_geometry);
	ClassDB::bind_method(D_METHOD("get_shape_transforms"), &Box3DReplay::get_shape_transforms);
}

} // namespace godot
