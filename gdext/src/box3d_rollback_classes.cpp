#include "box3d_rollback_classes.h"

#include <godot_cpp/core/class_db.hpp>

#include "box3d_raw.h"
#include "box3d_replay.h"
#include "box3d_rollback_session.h"
#include "box3d_rollback_world.h"

using namespace godot;

void godot::register_box3d_rollback_classes() {
	GDREGISTER_CLASS(Box3DBuffer);
	GDREGISTER_CLASS(Box3DRaw);
	GDREGISTER_CLASS(Box3DRollbackWorld);
	GDREGISTER_CLASS(Box3DRollbackSession);
	GDREGISTER_CLASS(Box3DReplay);
}
