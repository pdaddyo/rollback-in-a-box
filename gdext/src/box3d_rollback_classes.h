#pragma once

namespace godot {

// Register all reusable classes in the calling GDExtension's library context.
// Standalone builds call this from box3d_rollback_init; embedding projects call
// it from their own scene-level initializer.
void register_box3d_rollback_classes();

} // namespace godot
