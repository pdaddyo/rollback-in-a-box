// Copyright rollback-in-a-box. SPDX-License-Identifier: same as repository LICENSE.

#include "Box3DRollbackRawApi.h"
#include "Box3DRollbackModule.h"

extern "C" BOX3DROLLBACK_API const FBox3DRollbackRawApi* Box3DRollback_GetRawApiV1()
{
	static const FBox3DRollbackRawApi Api;
	return &Api;
}
