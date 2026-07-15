#include "rollback_shim.h"

#include "box3d/box3d.h"

// box3d internal headers — available because box3d is vendored and statically linked.
// This is the same machinery the recording system's keyframe ring uses
// (b3RecCaptureKeyframe / b3RecPlayerRestoreKeyframe in recording_replay.c).
#include "core.h"
#include "physics_world.h"
#include "recording.h"
#include "recording_replay.h"
#include "world_snapshot.h"

#include <stdlib.h>
#include <string.h>

struct B3RollbackCtx
{
	b3WorldId worldId;

	// Long-lived recording: owns the geometry registry that snapshots intern
	// hull/mesh geometry into. Never reset while the world lives.
	b3Recording* rec;

	// Ring of reusable snapshot buffers (buf.size = 0 reuses capacity).
	b3RecBuffer* slots;
	int* slotSizes; // -1 = empty
	int slotCount;

	// Slot view over rec->registry for restores: the exact inverse of
	// b3RecSeedKeyframeRegistry. bytes point at registry-owned blobs (not copies),
	// so entries stay valid for the life of rec. Grown append-only as the
	// registry grows; existing entries keep their lazily-created `live` objects.
	b3RegistrySlot* view;
	int viewCount;
	int viewCapacity;
};

static void b3r_RefreshView( B3RollbackCtx* ctx )
{
	const b3GeometryRegistry* reg = &ctx->rec->registry;
	if ( reg->count > ctx->viewCapacity )
	{
		int capacity = reg->count > 16 ? 2 * reg->count : 32;
		b3RegistrySlot* view = calloc( (size_t)capacity, sizeof( b3RegistrySlot ) );
		if ( ctx->view != NULL )
		{
			memcpy( view, ctx->view, (size_t)ctx->viewCount * sizeof( b3RegistrySlot ) );
			free( ctx->view );
		}
		ctx->view = view;
		ctx->viewCapacity = capacity;
	}

	for ( int i = ctx->viewCount; i < reg->count; ++i )
	{
		ctx->view[i].kind = reg->entries[i].kind;
		ctx->view[i].byteCount = reg->entries[i].byteCount;
		ctx->view[i].bytes = reg->entries[i].bytes;
		ctx->view[i].live = NULL;
	}
	ctx->viewCount = reg->count;
}

B3RollbackCtx* b3r_create( b3WorldId worldId, int slotCount )
{
	if ( slotCount <= 0 || b3World_IsValid( worldId ) == false )
	{
		return NULL;
	}

	B3RollbackCtx* ctx = calloc( 1, sizeof( B3RollbackCtx ) );
	ctx->worldId = worldId;
	ctx->rec = b3CreateRecording( 0 );
	ctx->slotCount = slotCount;
	ctx->slots = calloc( (size_t)slotCount, sizeof( b3RecBuffer ) );
	ctx->slotSizes = malloc( (size_t)slotCount * sizeof( int ) );
	for ( int i = 0; i < slotCount; ++i )
	{
		ctx->slotSizes[i] = -1;
	}
	return ctx;
}

void b3r_destroy( B3RollbackCtx* ctx )
{
	if ( ctx == NULL )
	{
		return;
	}

	for ( int i = 0; i < ctx->slotCount; ++i )
	{
		b3RecBufFree( &ctx->slots[i] );
	}
	free( ctx->slots );
	free( ctx->slotSizes );

	// Mirror b3RecFreeSlots for lazily-created compound objects, but do NOT free
	// view[i].bytes — those blobs belong to rec->registry and are freed by
	// b3DestroyRecording below.
	for ( int i = 0; i < ctx->viewCount; ++i )
	{
		if ( ctx->view[i].live != NULL && ctx->view[i].kind == b3_geometryCompound )
		{
			b3Free( ctx->view[i].live, (size_t)ctx->view[i].byteCount );
		}
	}
	free( ctx->view );

	b3DestroyRecording( ctx->rec );
	free( ctx );
}

int b3r_save( B3RollbackCtx* ctx, int slot )
{
	if ( ctx == NULL || slot < 0 || slot >= ctx->slotCount )
	{
		return -1;
	}

	b3World* world = b3GetWorldFromId( ctx->worldId );
	b3RecBuffer* buf = &ctx->slots[slot];
	buf->size = 0;
	int byteCount = b3SerializeWorld( world, buf, ctx->rec );

	// Serialize may have interned geometry it hadn't seen before.
	b3r_RefreshView( ctx );

	ctx->slotSizes[slot] = byteCount;
	return byteCount;
}

bool b3r_load( B3RollbackCtx* ctx, int slot )
{
	if ( ctx == NULL || slot < 0 || slot >= ctx->slotCount || ctx->slotSizes[slot] <= 0 )
	{
		return false;
	}

	b3World* world = b3GetWorldFromId( ctx->worldId );

	b3RecReader reader;
	memset( &reader, 0, sizeof( reader ) );
	reader.ok = true;
	reader.replayWorldId = ctx->worldId;
	reader.slots = ctx->view;
	reader.slotCount = ctx->viewCount;

	return b3DeserializeIntoShell( ctx->slots[slot].data, ctx->slotSizes[slot], world, &reader );
}

uint64_t b3r_world_hash( b3WorldId worldId )
{
	if ( b3World_IsValid( worldId ) == false )
	{
		return 0;
	}
	return b3HashWorldState( b3GetWorldFromId( worldId ) );
}

int b3r_slot_size( const B3RollbackCtx* ctx, int slot )
{
	if ( ctx == NULL || slot < 0 || slot >= ctx->slotCount )
	{
		return -1;
	}
	return ctx->slotSizes[slot];
}
