#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Box3DRollbackSimulation.generated.h"

// The rollback simulation contract, as a Blueprint-implementable interface.
//
// This mirrors rb::IRollbackSimulation. A game provides a UObject implementing
// these events (typically wrapping a UBox3DRollbackWorld plus game rules) and
// hands it to UBox3DRollbackSession::SetSimulationInterface. The session's
// native bridge forwards each rb::IRollbackSimulation call to the matching
// event here via Execute_*. (UBox3DRollbackWorld implements this directly, so
// simple games can also bind a world with SetSimulationWorld and skip the
// interface entirely.)
UINTERFACE(MinimalAPI, Blueprintable)
class UBox3DRollbackSimulation : public UInterface
{
	GENERATED_BODY()
};

class IBox3DRollbackSimulation
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Box3D Rollback")
	bool RollbackHasWorld();

	UFUNCTION(BlueprintNativeEvent, Category = "Box3D Rollback")
	int32 RollbackGetInputCount();

	UFUNCTION(BlueprintNativeEvent, Category = "Box3D Rollback")
	void RollbackInitSnapshots(int32 SlotCount);

	UFUNCTION(BlueprintNativeEvent, Category = "Box3D Rollback")
	bool RollbackSaveState(int32 Slot);

	UFUNCTION(BlueprintNativeEvent, Category = "Box3D Rollback")
	bool RollbackLoadState(int32 Slot);

	UFUNCTION(BlueprintNativeEvent, Category = "Box3D Rollback")
	int64 RollbackStateHash();

	UFUNCTION(BlueprintNativeEvent, Category = "Box3D Rollback")
	void RollbackStepFrame(const TArray<int64>& Inputs);

	// Return true only if the sim implements a meaningful RollbackBegin.
	UFUNCTION(BlueprintNativeEvent, Category = "Box3D Rollback")
	bool RollbackSupportsBegin();

	UFUNCTION(BlueprintNativeEvent, Category = "Box3D Rollback")
	void RollbackBegin(int64 TargetFrame, int32 WindowFrames, int32 PlayersMask);
};
