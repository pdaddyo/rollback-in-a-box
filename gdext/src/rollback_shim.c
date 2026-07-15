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

#include <float.h>
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

static uint64_t b3r_FnvFold( uint64_t h, uint64_t value )
{
	for ( int i = 0; i < 8; ++i )
	{
		h ^= ( value >> ( 8 * i ) ) & 0xFF;
		h *= 0x100000001B3ULL;
	}
	return h;
}

// Canonical scene: mixed shape families in contact, stepped long enough for
// stacking, sliding, and sleep transitions to exercise the solver paths whose
// float behavior differs across compilers and SIMD widths.
static uint64_t b3r_SimulationProbe( void )
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.workerCount = 1;
	b3WorldId worldId = b3CreateWorld( &worldDef );

	{
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.position = ( b3Vec3 ){ 0.0f, -1.0f, 0.0f };
		b3BodyId ground = b3CreateBody( worldId, &bodyDef );
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull hull = b3MakeBoxHull( 20.0f, 1.0f, 20.0f );
		b3CreateHullShape( ground, &shapeDef, &hull.base );
	}

	for ( int i = 0; i < 6; ++i )
	{
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = ( b3Vec3 ){ 0.05f * (float)i, 0.5f + 1.05f * (float)i, 0.02f * (float)i };
		b3BodyId body = b3CreateBody( worldId, &bodyDef );
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.density = 300.0f;
		b3BoxHull hull = b3MakeBoxHull( 0.5f, 0.5f, 0.5f );
		b3CreateHullShape( body, &shapeDef, &hull.base );
	}

	{
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = ( b3Vec3 ){ 2.5f, 4.0f, 0.0f };
		bodyDef.linearVelocity = ( b3Vec3 ){ -1.5f, 0.0f, 0.75f };
		b3BodyId body = b3CreateBody( worldId, &bodyDef );
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.density = 300.0f;
		b3Sphere sphere = { { 0.0f, 0.0f, 0.0f }, 0.5f };
		b3CreateSphereShape( body, &shapeDef, &sphere );
	}

	{
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_dynamicBody;
		bodyDef.position = ( b3Vec3 ){ -2.0f, 2.0f, 1.0f };
		b3BodyId body = b3CreateBody( worldId, &bodyDef );
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.density = 300.0f;
		b3Capsule capsule = { { 0.0f, -0.4f, 0.0f }, { 0.0f, 0.4f, 0.0f }, 0.3f };
		b3CreateCapsuleShape( body, &shapeDef, &capsule );
	}

	uint64_t h = 0xCBF29CE484222325ULL;
	for ( int frame = 0; frame < 90; ++frame )
	{
		b3World_Step( worldId, 1.0f / 60.0f, 4 );
		if ( ( frame + 1 ) % 30 == 0 )
		{
			h = b3r_FnvFold( h, b3HashWorldState( b3GetWorldFromId( worldId ) ) );
		}
	}

	b3DestroyWorld( worldId );
	return h;
}

uint64_t b3r_determinism_fingerprint( void )
{
	static uint64_t cached = 0;
	if ( cached != 0 )
	{
		return cached;
	}

	uint64_t h = 0xCBF29CE484222325ULL;

	b3Version version = b3GetVersion();
	h = b3r_FnvFold( h, (uint64_t)version.major << 32 | (uint64_t)version.minor << 16 | (uint64_t)version.revision );

	h = b3r_FnvFold( h, (uint64_t)B3_SIMD_WIDTH );
#if defined( B3_SIMD_SSE2 )
	h = b3r_FnvFold( h, 1 );
#elif defined( B3_SIMD_NEON )
	h = b3r_FnvFold( h, 2 );
#elif defined( B3_SIMD_NONE )
	h = b3r_FnvFold( h, 3 );
#else
	h = b3r_FnvFold( h, 4 );
#endif
	h = b3r_FnvFold( h, (uint64_t)(int64_t)FLT_EVAL_METHOD );

	// Snapshot images are raw struct memory: layout drift is a compatibility break.
	h = b3r_FnvFold( h, (uint64_t)sizeof( b3World ) );
	h = b3r_FnvFold( h, (uint64_t)sizeof( b3WorldDef ) );
	h = b3r_FnvFold( h, (uint64_t)sizeof( b3Transform ) );

	h = b3r_FnvFold( h, b3r_SimulationProbe() );

	cached = h != 0 ? h : 1;
	return cached;
}
