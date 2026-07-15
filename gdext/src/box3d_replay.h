// Box3DReplay — plays back a box3d physics recording (.b3r) in its own
// private world (via box3d's b3RecPlayer) and exposes shape geometry plus
// per-frame body transforms so a viewer scene can render the replay.
// Read-only debugging/showcase tool: nothing here touches the game sim.
#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int64_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>

#include "box3d/box3d.h"

#include <vector>

namespace godot {

class Box3DReplay : public Node3D {
	GDCLASS(Box3DReplay, Node3D)

	b3RecPlayer *player = nullptr;

	// Shapes of the replay world, collected each frame in a stable order.
	std::vector<b3ShapeId> collect_shapes() const;

protected:
	static void _bind_methods();

public:
	~Box3DReplay() override;

	bool load_file(const String &path);
	void close();
	bool is_loaded() const { return player != nullptr; }

	Dictionary get_info() const; // frame_count, time_step, sub_steps, bounds_min/max
	bool step_frame(); // false at end of recording
	void seek_frame(int frame);
	void restart();
	int get_frame() const;
	int get_frame_count() const;
	bool is_at_end() const;
	bool has_diverged() const;
	int get_diverge_frame() const;

	// Rendering interface. Ids are stable per shape ((index1 << 32) | generation);
	// the id list changes only when the recording creates/destroys bodies, so a
	// viewer rebuilds meshes when it changes and otherwise just applies transforms.
	PackedInt64Array get_shape_ids() const;
	// {type: "sphere"|"capsule"|"hull"|"bounds", dynamic: bool, ...geometry}
	Dictionary get_shape_geometry(int64_t packed_id) const;
	PackedFloat32Array get_shape_transforms() const; // 7 floats per shape: px py pz qx qy qz qw
};

} // namespace godot
