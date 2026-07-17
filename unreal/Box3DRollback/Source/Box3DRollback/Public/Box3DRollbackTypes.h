#pragma once

#include "CoreMinimal.h"
#include "Box3DRollbackTypes.generated.h"

// Blueprint-visible mirror of rb::RollbackScope (the diagnostic scope of the
// last partial resimulation). Populated by UBox3DRollbackWorld::RollbackBeginScope.
USTRUCT(BlueprintType)
struct FBox3DRollbackScope
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Box3D Rollback")
	bool bValid = false;

	UPROPERTY(BlueprintReadOnly, Category = "Box3D Rollback")
	int64 TargetFrame = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Box3D Rollback")
	int32 Window = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Box3D Rollback")
	int32 MispredictedMask = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Box3D Rollback")
	int32 AffectedBodies = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Box3D Rollback")
	int32 AwakeBodies = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Box3D Rollback")
	int32 TotalBodies = 0;
};
