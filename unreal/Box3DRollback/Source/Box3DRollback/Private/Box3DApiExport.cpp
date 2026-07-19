// Copyright rollback-in-a-box. SPDX-License-Identifier: same as repository LICENSE.
//
// Re-export the Box3D C API from this module's DLL.
//
// Box3D lives in a prebuilt STATIC library. In Unreal's modular (editor)
// builds, any game module that also linked that static lib would get its own
// private copy of Box3D's process-global state — the b3_worlds array, the
// length-units global, the assert hook. A world created through this plugin
// would then be invisible to raw b3* calls made from the game module (null
// deref / silent nonsense), and b3SetLengthUnitsPerMeter would set the wrong
// copy's global.
//
// Box3D supports being folded into another shared library by predefining
// BOX3D_EXPORT (see box3d/base.h). This module compiles with
// BOX3D_EXPORT=__declspec(dllexport) (Build.cs, Win64): including the full
// API headers here embeds linker /EXPORT directives for every B3_API
// function, and the linker pulls the definitions from the static lib into
// this DLL's export table. Game modules compile with
// BOX3D_EXPORT=__declspec(dllimport) and resolve against this DLL — ONE copy
// of Box3D per process, exactly as the raw-API escape hatch assumes.

#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "box3d/math_functions.h"
