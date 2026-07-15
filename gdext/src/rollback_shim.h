// C shim over box3d's internal world-snapshot machinery (src/world_snapshot.h).
// Kept in C so the internal C headers never enter a C++ translation unit.
//
// Lifetime contract: the context owns the geometry registry that restored
// mesh/heightfield shapes point into — destroy the world BEFORE the context.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "box3d/id.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct B3RollbackCtx B3RollbackCtx;

// Create a rollback context with slotCount reusable snapshot buffers.
B3RollbackCtx* b3r_create( b3WorldId worldId, int slotCount );
void b3r_destroy( B3RollbackCtx* ctx );

// Serialize the current world state into a slot. Returns the byte count, -1 on error.
// Buffers are reused across saves; no allocation churn after warm-up.
int b3r_save( B3RollbackCtx* ctx, int slot );

// Restore the world in place from a slot. World id and snapshot-era body handles
// stay valid. Returns false if the slot is empty or the image is incompatible.
bool b3r_load( B3RollbackCtx* ctx, int slot );

// FNV-1a over raw float bits of every live body transform + awake velocities.
// Bitwise-comparable across peers running the same binary. Works without a
// rollback context so tests can hash a plain world.
uint64_t b3r_world_hash( b3WorldId worldId );

int b3r_slot_size( const B3RollbackCtx* ctx, int slot );

// Determinism fingerprint of this binary. Folds the Box3D version, SIMD
// flavor and width, the compiler's float evaluation mode, snapshot-relevant
// struct layouts, and the hashed result of a canonical simulation probe
// (a fixed scene stepped 90 frames). Two builds with equal fingerprints
// produced bit-identical results on the probe; builds with different
// fingerprints must not be assumed to resimulate each other's frames.
// Computed once and cached; the probe world is created and destroyed inside.
uint64_t b3r_determinism_fingerprint( void );

#ifdef __cplusplus
}
#endif
