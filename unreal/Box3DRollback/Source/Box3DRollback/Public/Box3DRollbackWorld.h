#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Box3DRollbackTypes.h"
#include "Box3DRollbackSimulation.h"
#include "Box3DRollbackWorld.generated.h"

// Neutral core, forward-declared so this header never pulls in Box3D headers
// (UHT only needs the reflected surface). The definition lives in the .cpp.
namespace rb { class RollbackWorld; }

// Godot's Box3DRollbackWorld, ported to Unreal as a thin UObject adapter over
// rb::RollbackWorld. Owns a Box3D world, fixed stepping, snapshots, hashes,
// transforms, and recording. Game rules belong in a simulation on top.
//
// It implements IBox3DRollbackSimulation, so a session can drive it directly.
UCLASS(BlueprintType, Blueprintable)
class BOX3DROLLBACK_API UBox3DRollbackWorld : public UObject, public IBox3DRollbackSimulation
{
	GENERATED_BODY()

public:
	UBox3DRollbackWorld();
	virtual ~UBox3DRollbackWorld() override;
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|World")
	void CreateWorld();
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|World")
	void DestroyWorld();
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|World")
	bool HasWorld() const;

	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Bodies")
	int32 AddStaticBox(FVector Position, FVector HalfExtents, float Friction = 0.6f);
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Bodies")
	int32 AddDynamicBox(FVector Position, FVector HalfExtents, float Density = 300.0f, float Friction = 0.6f);
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Bodies")
	int32 AddStaticSphere(FVector Position, float Radius, float Friction = 0.6f);
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Bodies")
	int32 AddDynamicSphere(FVector Position, float Radius, float Density = 300.0f, float Friction = 0.6f);
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Bodies")
	int32 AddStaticCapsule(FVector Position, FVector PointA, FVector PointB, float Radius, float Friction = 0.6f);
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Bodies")
	int32 AddDynamicCapsule(FVector Position, FVector PointA, FVector PointB, float Radius, float Density = 300.0f, float Friction = 0.6f);

	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Bodies")
	void SetBodyLinearVelocity(int32 Handle, FVector Velocity);
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Bodies")
	FVector GetBodyVelocity(int32 Handle) const;
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Bodies")
	void ApplyBodyLinearImpulse(int32 Handle, FVector Impulse, bool bWake = true);

	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Step")
	void StepFrame(const TArray<int64>& Inputs);
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Step")
	int64 GetFrame() const;

	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Snapshots")
	void InitSnapshots(int32 SlotCount);
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Snapshots")
	bool SaveState(int32 Slot);
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Snapshots")
	bool LoadState(int32 Slot);
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Snapshots")
	int64 StateHash() const;

	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Bodies")
	int32 GetBodyCount() const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Bodies")
	int32 GetLiveBodyCount() const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Bodies")
	int32 GetAwakeBodyCount() const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Bodies")
	int64 GetBodyId(int32 Handle) const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|World")
	int64 GetWorldId() const;

	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Rollback")
	void SetPlayerBodies(int32 Player, const TArray<int64>& Handles);
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Rollback")
	TArray<int64> ComputeAffectedBodies(int32 PlayersMask, int32 WindowFrames) const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Rollback")
	FBox3DRollbackScope GetLastRollbackScope() const;
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Rollback")
	void RollbackBeginScope(int64 TargetFrame, int32 WindowFrames, int32 PlayersMask);

	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Render")
	FTransform GetBodyTransform(int32 Handle) const;
	// Flat transform buffer, 7 floats per body: px py pz qx qy qz qw.
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Render")
	TArray<float> GetTransforms() const;
	// Flat meta buffer, 4 floats per body: x/y/z extents (or radius), kind.
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Render")
	TArray<float> GetBodyMeta() const;

	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Recording")
	bool StartRecording();
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Recording")
	void StopRecording();
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Recording")
	bool IsRecording() const;
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Recording")
	bool SaveRecording(const FString& Path);
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Recording")
	static bool ValidateRecordingFile(const FString& Path);

	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Config")
	void SetWorkerCount(int32 Count);
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Config")
	int32 GetWorkerCount() const;
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Config")
	void SetInputCount(int32 Count);
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Config")
	int32 GetInputCount() const;
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Config")
	void SetTimeStep(float Value);
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Config")
	float GetTimeStep() const;
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback|Config")
	void SetSubSteps(int32 Count);
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Config")
	int32 GetSubSteps() const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback|Config")
	float GetLastStepTimeMs() const;

	// --- IBox3DRollbackSimulation (a world is itself a valid simulation) ---
	virtual bool RollbackHasWorld_Implementation() override;
	virtual int32 RollbackGetInputCount_Implementation() override;
	virtual void RollbackInitSnapshots_Implementation(int32 SlotCount) override;
	virtual bool RollbackSaveState_Implementation(int32 Slot) override;
	virtual bool RollbackLoadState_Implementation(int32 Slot) override;
	virtual int64 RollbackStateHash_Implementation() override;
	virtual void RollbackStepFrame_Implementation(const TArray<int64>& Inputs) override;
	virtual bool RollbackSupportsBegin_Implementation() override;
	virtual void RollbackBegin_Implementation(int64 TargetFrame, int32 WindowFrames, int32 PlayersMask) override;

	// C++-only access to the neutral core, so UBox3DRollbackSession can bind the
	// world directly (rb::RollbackWorld implements rb::IRollbackSimulation) with
	// no reflection round-trip.
	rb::RollbackWorld* GetCore() const { return Core; }

private:
	// Raw owning pointer (freed in the destructor / BeginDestroy, where the type
	// is complete). A TUniquePtr member would force UHT's generated vtable-helper
	// constructor to instantiate the deleter on the forward-declared type.
	rb::RollbackWorld* Core = nullptr;
};
