#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Box3DRollbackSession.generated.h"

class UBox3DRollbackWorld;

// Fired the first time a peer's confirmed-frame hash disagrees with ours.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBox3DDesyncDetected, int64, Frame);
// Fired the first time a peer with an incompatible determinism fingerprint is seen.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBox3DPeerIncompatible, int32, Player, int64, Fingerprint);

// Godot's Box3DRollbackSession, ported to Unreal as a thin UObject adapter over
// rb::RollbackSession. All prediction/rollback/netcode logic is in the neutral
// core; this class marshals UE types and turns core edge events into delegates.
//
// Transport is the game's concern: forward the opaque bytes from GetPacket()
// over any channel, and feed received bytes to IngestPacket().
UCLASS(BlueprintType, Blueprintable)
class BOX3DROLLBACK_API UBox3DRollbackSession : public UObject
{
	GENERATED_BODY()

public:
	UBox3DRollbackSession();
	virtual ~UBox3DRollbackSession() override;
	virtual void BeginDestroy() override;

	// Fast path: bind a world directly (rb::RollbackWorld implements the sim
	// interface natively, so no per-call reflection). Most games that don't add
	// custom step logic use this.
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback")
	void SetSimulationWorld(UBox3DRollbackWorld* World);

	// General path: bind any UObject implementing IBox3DRollbackSimulation.
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback")
	void SetSimulationInterface(UObject* Simulation);

	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback")
	void Configure(int32 LocalPlayer, int32 NumPlayers, int32 InputDelay = 2, int32 MaxPrediction = 8);
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback")
	void Start();

	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback")
	bool Tick(int64 LocalInput);
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback")
	TArray<uint8> GetPacket();
	UFUNCTION(BlueprintCallable, Category = "Box3D Rollback")
	void IngestPacket(const TArray<uint8>& Packet);

	UFUNCTION(BlueprintPure, Category = "Box3D Rollback")
	int64 GetCurrentFrame() const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback")
	int64 GetConfirmedFrame() const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback")
	int64 GetSafeFrame() const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback")
	bool IsStalled() const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback")
	bool IsDesynced() const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback")
	int64 GetDesyncFrame() const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback")
	int32 GetLastRollbackDepth() const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback")
	int32 GetLastMispredictedMask() const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback")
	int64 GetTotalRollbackFrames() const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback")
	int64 GetTotalStalledTicks() const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback")
	float GetFrameAdvantage() const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback")
	int64 GetHashForFrame(int64 Frame) const;
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback")
	int32 GetIncompatiblePeerMask() const;

	// Determinism fingerprint of this binary. Compare across peers during
	// matchmaking; the session also rejects packets from mismatched fingerprints.
	UFUNCTION(BlueprintPure, Category = "Box3D Rollback")
	static int64 GetBuildFingerprint();

	UPROPERTY(BlueprintAssignable, Category = "Box3D Rollback")
	FBox3DDesyncDetected OnDesyncDetected;
	UPROPERTY(BlueprintAssignable, Category = "Box3D Rollback")
	FBox3DPeerIncompatible OnPeerIncompatible;

private:
	// rb::RollbackSession + the bridges live here so no Box3D/neutral headers
	// leak into this UObject header (UHT only sees the reflected surface). Raw
	// owning pointer (freed in the destructor, where FImpl is complete); a
	// TUniquePtr member would make UHT's vtable-helper ctor delete an incomplete
	// type.
	struct FImpl;
	FImpl* Impl = nullptr;

	// Keeps the bound simulation UObject referenced so GC does not collect it
	// out from under an active session.
	UPROPERTY()
	TObjectPtr<UObject> BoundSimulation;
};
