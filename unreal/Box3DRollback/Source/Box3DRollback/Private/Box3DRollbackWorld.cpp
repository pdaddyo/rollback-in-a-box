#include "Box3DRollbackWorld.h"

#include "box3d_rollback/rb_world.h"

#include <vector>

namespace
{
	rb::Vec3 ToRb(const FVector& V)
	{
		return rb::Vec3{ (float)V.X, (float)V.Y, (float)V.Z };
	}

	FVector ToUE(const rb::Vec3& V)
	{
		return FVector(V.x, V.y, V.z);
	}
}

UBox3DRollbackWorld::UBox3DRollbackWorld()
{
	Core = new rb::RollbackWorld();
}

UBox3DRollbackWorld::~UBox3DRollbackWorld()
{
	delete Core;
	Core = nullptr;
}

void UBox3DRollbackWorld::BeginDestroy()
{
	delete Core;
	Core = nullptr;
	Super::BeginDestroy();
}

void UBox3DRollbackWorld::CreateWorld() { Core->create_world(); }
void UBox3DRollbackWorld::DestroyWorld() { Core->destroy_world(); }
bool UBox3DRollbackWorld::HasWorld() const { return Core->has_world(); }

int32 UBox3DRollbackWorld::AddStaticBox(FVector Position, FVector HalfExtents, float Friction)
{
	return Core->add_static_box(ToRb(Position), ToRb(HalfExtents), Friction);
}

int32 UBox3DRollbackWorld::AddStaticCompoundBoxes(const TArray<FVector>& Positions,
	const TArray<FVector>& HalfExtents, float Friction)
{
	// Core clones and retains the immutable compound for the lifetime of this world.
	if (Positions.IsEmpty() || Positions.Num() != HalfExtents.Num())
	{
		return -1;
	}
	std::vector<rb::Vec3> RbPositions;
	std::vector<rb::Vec3> RbHalfExtents;
	RbPositions.reserve(Positions.Num());
	RbHalfExtents.reserve(HalfExtents.Num());
	for (int32 Index = 0; Index < Positions.Num(); ++Index)
	{
		RbPositions.push_back(ToRb(Positions[Index]));
		RbHalfExtents.push_back(ToRb(HalfExtents[Index]));
	}
	return Core->add_static_compound_boxes(RbPositions.data(), RbHalfExtents.data(),
		Positions.Num(), Friction);
}

int32 UBox3DRollbackWorld::AddDynamicBox(FVector Position, FVector HalfExtents, float Density, float Friction)
{
	return Core->add_dynamic_box(ToRb(Position), ToRb(HalfExtents), Density, Friction);
}

int32 UBox3DRollbackWorld::AddStaticSphere(FVector Position, float Radius, float Friction)
{
	return Core->add_static_sphere(ToRb(Position), Radius, Friction);
}

int32 UBox3DRollbackWorld::AddDynamicSphere(FVector Position, float Radius, float Density, float Friction)
{
	return Core->add_dynamic_sphere(ToRb(Position), Radius, Density, Friction);
}

int32 UBox3DRollbackWorld::AddStaticCapsule(FVector Position, FVector PointA, FVector PointB, float Radius, float Friction)
{
	return Core->add_static_capsule(ToRb(Position), ToRb(PointA), ToRb(PointB), Radius, Friction);
}

int32 UBox3DRollbackWorld::AddDynamicCapsule(FVector Position, FVector PointA, FVector PointB, float Radius, float Density, float Friction)
{
	return Core->add_dynamic_capsule(ToRb(Position), ToRb(PointA), ToRb(PointB), Radius, Density, Friction);
}

bool UBox3DRollbackWorld::ResolveCharacterMover(int32 Handle, FVector StartPosition,
	float HalfHeight, float Radius, uint64 QueryCategoryBits, uint64 QueryMaskBits,
	bool& OutGrounded, int32& OutPlaneCount, int32& OutSolverIterations)
{
	const rb::CharacterMoverResult Result = Core->resolve_character_mover(Handle, ToRb(StartPosition),
		HalfHeight, Radius, QueryCategoryBits, QueryMaskBits);
	OutGrounded = Result.grounded;
	OutPlaneCount = Result.plane_count;
	OutSolverIterations = Result.solver_iterations;
	return Result.valid;
}

void UBox3DRollbackWorld::SetBodyLinearVelocity(int32 Handle, FVector Velocity)
{
	Core->set_body_linear_velocity(Handle, ToRb(Velocity));
}

FVector UBox3DRollbackWorld::GetBodyVelocity(int32 Handle) const
{
	return ToUE(Core->get_body_velocity(Handle));
}

void UBox3DRollbackWorld::ApplyBodyLinearImpulse(int32 Handle, FVector Impulse, bool bWake)
{
	Core->apply_body_linear_impulse(Handle, ToRb(Impulse), bWake);
}

void UBox3DRollbackWorld::StepFrame(const TArray<int64>& Inputs)
{
	Core->step_frame(Inputs.GetData(), Inputs.Num());
}

int64 UBox3DRollbackWorld::GetFrame() const { return (int64)Core->get_frame(); }

void UBox3DRollbackWorld::InitSnapshots(int32 SlotCount) { Core->init_snapshots(SlotCount); }
bool UBox3DRollbackWorld::SaveState(int32 Slot) { return Core->save_state(Slot); }
bool UBox3DRollbackWorld::LoadState(int32 Slot) { return Core->load_state(Slot); }
int64 UBox3DRollbackWorld::StateHash() const { return (int64)Core->get_state_hash(); }

int32 UBox3DRollbackWorld::GetBodyCount() const { return Core->get_body_count(); }
int32 UBox3DRollbackWorld::GetLiveBodyCount() const { return Core->get_live_body_count(); }
int32 UBox3DRollbackWorld::GetAwakeBodyCount() const { return Core->get_awake_body_count(); }
int64 UBox3DRollbackWorld::GetBodyId(int32 Handle) const { return Core->get_body_id(Handle); }
int64 UBox3DRollbackWorld::GetWorldId() const { return Core->get_world_id(); }

void UBox3DRollbackWorld::SetPlayerBodies(int32 Player, const TArray<int64>& Handles)
{
	Core->set_player_bodies(Player, Handles.GetData(), Handles.Num());
}

TArray<int64> UBox3DRollbackWorld::ComputeAffectedBodies(int32 PlayersMask, int32 WindowFrames) const
{
	const std::vector<int64_t> Affected = Core->compute_affected_bodies(PlayersMask, WindowFrames);
	TArray<int64> Out;
	Out.Reserve((int32)Affected.size());
	for (int64_t Id : Affected)
	{
		Out.Add((int64)Id);
	}
	return Out;
}

FBox3DRollbackScope UBox3DRollbackWorld::GetLastRollbackScope() const
{
	const rb::RollbackScope& S = Core->get_last_rollback_scope();
	FBox3DRollbackScope Out;
	Out.bValid = S.valid;
	Out.TargetFrame = S.target_frame;
	Out.Window = S.window;
	Out.MispredictedMask = S.mispredicted_mask;
	Out.AffectedBodies = S.affected_bodies;
	Out.AwakeBodies = S.awake_bodies;
	Out.TotalBodies = S.total_bodies;
	return Out;
}

void UBox3DRollbackWorld::RollbackBeginScope(int64 TargetFrame, int32 WindowFrames, int32 PlayersMask)
{
	Core->rollback_begin(TargetFrame, WindowFrames, PlayersMask);
}

FTransform UBox3DRollbackWorld::GetBodyTransform(int32 Handle) const
{
	const rb::Transform Xf = Core->get_body_transform(Handle);
	return FTransform(FQuat(Xf.q.x, Xf.q.y, Xf.q.z, Xf.q.w), FVector(Xf.p.x, Xf.p.y, Xf.p.z));
}

TArray<float> UBox3DRollbackWorld::GetTransforms() const
{
	const std::vector<float> V = Core->get_transforms();
	TArray<float> Out;
	if (!V.empty())
	{
		Out.Append(V.data(), (int32)V.size());
	}
	return Out;
}

TArray<float> UBox3DRollbackWorld::GetBodyMeta() const
{
	const std::vector<float>& V = Core->get_body_meta();
	TArray<float> Out;
	if (!V.empty())
	{
		Out.Append(V.data(), (int32)V.size());
	}
	return Out;
}

bool UBox3DRollbackWorld::StartRecording() { return Core->start_recording(); }
void UBox3DRollbackWorld::StopRecording() { Core->stop_recording(); }
bool UBox3DRollbackWorld::IsRecording() const { return Core->is_recording(); }

bool UBox3DRollbackWorld::SaveRecording(const FString& Path)
{
	return Core->save_recording(TCHAR_TO_UTF8(*Path));
}

bool UBox3DRollbackWorld::ValidateRecordingFile(const FString& Path)
{
	return rb::RollbackWorld::validate_recording_file(TCHAR_TO_UTF8(*Path));
}

void UBox3DRollbackWorld::SetWorkerCount(int32 Count) { Core->set_worker_count(Count); }
int32 UBox3DRollbackWorld::GetWorkerCount() const { return Core->get_worker_count(); }
void UBox3DRollbackWorld::SetInputCount(int32 Count) { Core->set_input_count(Count); }
int32 UBox3DRollbackWorld::GetInputCount() const { return Core->get_input_count(); }
void UBox3DRollbackWorld::SetTimeStep(float Value) { Core->set_time_step(Value); }
float UBox3DRollbackWorld::GetTimeStep() const { return Core->get_time_step(); }
void UBox3DRollbackWorld::SetSubSteps(int32 Count) { Core->set_sub_steps(Count); }
int32 UBox3DRollbackWorld::GetSubSteps() const { return Core->get_sub_steps(); }
float UBox3DRollbackWorld::GetLastStepTimeMs() const { return (float)Core->get_last_step_time_ms(); }

// --- IBox3DRollbackSimulation ---

bool UBox3DRollbackWorld::RollbackHasWorld_Implementation() { return Core->has_world(); }
int32 UBox3DRollbackWorld::RollbackGetInputCount_Implementation() { return Core->get_input_count(); }
void UBox3DRollbackWorld::RollbackInitSnapshots_Implementation(int32 SlotCount) { Core->init_snapshots(SlotCount); }
bool UBox3DRollbackWorld::RollbackSaveState_Implementation(int32 Slot) { return Core->save_state(Slot); }
bool UBox3DRollbackWorld::RollbackLoadState_Implementation(int32 Slot) { return Core->load_state(Slot); }
int64 UBox3DRollbackWorld::RollbackStateHash_Implementation() { return (int64)Core->get_state_hash(); }

void UBox3DRollbackWorld::RollbackStepFrame_Implementation(const TArray<int64>& Inputs)
{
	Core->step_frame(Inputs.GetData(), Inputs.Num());
}

bool UBox3DRollbackWorld::RollbackSupportsBegin_Implementation() { return true; }

void UBox3DRollbackWorld::RollbackBegin_Implementation(int64 TargetFrame, int32 WindowFrames, int32 PlayersMask)
{
	Core->rollback_begin(TargetFrame, WindowFrames, PlayersMask);
}
