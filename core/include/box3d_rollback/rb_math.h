// Engine-neutral POD math types for the Box3D rollback core.
//
// These deliberately carry no engine or Box3D types so the core public API
// stays free of both godot-cpp and UE headers. Adapters convert to/from their
// engine's own Vector3/Transform3D (Godot) or FVector/FTransform (Unreal).
#pragma once

namespace rb {

struct Vec3 {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

// Quaternion with (x, y, z) vector part and w scalar. Matches Box3D's b3Quat
// ordering (q.v.x/y/z, q.s) so the conversion in the core .cpp is a plain copy.
struct Quat {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;
};

struct Transform {
	Vec3 p;
	Quat q;
};

} // namespace rb
