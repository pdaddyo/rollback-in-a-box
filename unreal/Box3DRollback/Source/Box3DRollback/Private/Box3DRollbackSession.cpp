#include "Box3DRollbackSession.h"

#include "Box3DRollbackSimulation.h"
#include "Box3DRollbackWorld.h"

#include "box3d_rollback/rb_session.h"
#include "box3d_rollback/rb_simulation.h"
#include "box3d_rollback/rb_world.h"

#include <vector>

namespace
{
	// Forwards rb::IRollbackSimulation calls to a UObject implementing
	// IBox3DRollbackSimulation, via the UHT-generated Execute_* thunks. Uses a
	// weak pointer so a garbage-collected sim reports itself dead.
	class FUnrealSimBridge : public rb::IRollbackSimulation
	{
	public:
		TWeakObjectPtr<UObject> SimObject;

		UObject* Resolve() const { return SimObject.Get(); }

		bool is_alive() const override { return SimObject.IsValid(); }

		bool has_world() const override
		{
			UObject* O = Resolve();
			return O != nullptr && IBox3DRollbackSimulation::Execute_RollbackHasWorld(O);
		}

		int get_input_count() const override
		{
			UObject* O = Resolve();
			return O != nullptr ? IBox3DRollbackSimulation::Execute_RollbackGetInputCount(O) : 0;
		}

		void init_snapshots(int slot_count) override
		{
			if (UObject* O = Resolve())
			{
				IBox3DRollbackSimulation::Execute_RollbackInitSnapshots(O, slot_count);
			}
		}

		bool save_state(int slot) override
		{
			UObject* O = Resolve();
			return O != nullptr && IBox3DRollbackSimulation::Execute_RollbackSaveState(O, slot);
		}

		bool load_state(int slot) override
		{
			UObject* O = Resolve();
			return O != nullptr && IBox3DRollbackSimulation::Execute_RollbackLoadState(O, slot);
		}

		uint64_t get_state_hash() const override
		{
			UObject* O = Resolve();
			return O != nullptr ? (uint64_t)IBox3DRollbackSimulation::Execute_RollbackStateHash(O) : 0;
		}

		void step_frame(const int64_t* inputs, int count) override
		{
			UObject* O = Resolve();
			if (O == nullptr)
			{
				return;
			}
			TArray<int64> Arr;
			Arr.SetNumUninitialized(count);
			for (int i = 0; i < count; ++i)
			{
				Arr[i] = inputs[i];
			}
			IBox3DRollbackSimulation::Execute_RollbackStepFrame(O, Arr);
		}

		bool supports_rollback_begin() const override
		{
			UObject* O = Resolve();
			return O != nullptr && IBox3DRollbackSimulation::Execute_RollbackSupportsBegin(O);
		}

		void rollback_begin(int64_t target_frame, int window_frames, int players_mask) override
		{
			if (UObject* O = Resolve())
			{
				IBox3DRollbackSimulation::Execute_RollbackBegin(O, target_frame, window_frames, players_mask);
			}
		}
	};

	// Turns rb::RollbackSession edge events into the session's dynamic delegates.
	class FUnrealSessionObserver : public rb::ISessionObserver
	{
	public:
		TWeakObjectPtr<UBox3DRollbackSession> Owner;

		void on_desync(int64_t frame) override
		{
			if (UBox3DRollbackSession* O = Owner.Get())
			{
				O->OnDesyncDetected.Broadcast((int64)frame);
			}
		}

		void on_peer_incompatible(int player, int64_t fingerprint) override
		{
			if (UBox3DRollbackSession* O = Owner.Get())
			{
				O->OnPeerIncompatible.Broadcast(player, (int64)fingerprint);
			}
		}
	};
}

struct UBox3DRollbackSession::FImpl
{
	rb::RollbackSession Session;
	FUnrealSimBridge Bridge;
	FUnrealSessionObserver Observer;
};

UBox3DRollbackSession::UBox3DRollbackSession()
{
	Impl = new FImpl();
	Impl->Observer.Owner = this;
	Impl->Session.set_simulation(&Impl->Bridge);
	Impl->Session.set_observer(&Impl->Observer);
}

UBox3DRollbackSession::~UBox3DRollbackSession()
{
	delete Impl;
	Impl = nullptr;
}

void UBox3DRollbackSession::BeginDestroy()
{
	Super::BeginDestroy();
}

void UBox3DRollbackSession::SetSimulationWorld(UBox3DRollbackWorld* World)
{
	if (Impl->Session.is_active())
	{
		return;
	}
	BoundSimulation = World;
	Impl->Bridge.SimObject = nullptr;
	rb::IRollbackSimulation* Sim = (World != nullptr)
		? static_cast<rb::IRollbackSimulation*>(World->GetCore())
		: static_cast<rb::IRollbackSimulation*>(&Impl->Bridge);
	Impl->Session.set_simulation(Sim);
}

void UBox3DRollbackSession::SetSimulationInterface(UObject* Simulation)
{
	if (Impl->Session.is_active())
	{
		return;
	}
	if (Simulation != nullptr && !Simulation->Implements<UBox3DRollbackSimulation>())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Box3DRollbackSession: bound object does not implement IBox3DRollbackSimulation; ignoring."));
		Simulation = nullptr;
	}
	BoundSimulation = Simulation;
	Impl->Bridge.SimObject = Simulation;
	Impl->Session.set_simulation(&Impl->Bridge);
}

void UBox3DRollbackSession::Configure(int32 LocalPlayer, int32 NumPlayers, int32 InputDelay, int32 MaxPrediction)
{
	Impl->Session.configure(LocalPlayer, NumPlayers, InputDelay, MaxPrediction);
}

void UBox3DRollbackSession::Start()
{
	Impl->Session.start();
}

bool UBox3DRollbackSession::Tick(int64 LocalInput)
{
	return Impl->Session.tick(LocalInput);
}

TArray<uint8> UBox3DRollbackSession::GetPacket()
{
	const std::vector<uint8_t> Bytes = Impl->Session.get_packet();
	TArray<uint8> Out;
	if (!Bytes.empty())
	{
		Out.Append(Bytes.data(), (int32)Bytes.size());
	}
	return Out;
}

void UBox3DRollbackSession::IngestPacket(const TArray<uint8>& Packet)
{
	Impl->Session.ingest_packet(Packet.GetData(), (size_t)Packet.Num());
}

int64 UBox3DRollbackSession::GetCurrentFrame() const { return Impl->Session.get_current_frame(); }
int64 UBox3DRollbackSession::GetConfirmedFrame() const { return Impl->Session.get_confirmed_frame(); }
int64 UBox3DRollbackSession::GetSafeFrame() const { return Impl->Session.get_safe_frame(); }
bool UBox3DRollbackSession::IsStalled() const { return Impl->Session.is_stalled(); }
bool UBox3DRollbackSession::IsDesynced() const { return Impl->Session.is_desynced(); }
int64 UBox3DRollbackSession::GetDesyncFrame() const { return Impl->Session.get_desync_frame(); }
int32 UBox3DRollbackSession::GetLastRollbackDepth() const { return Impl->Session.get_last_rollback_depth(); }
int32 UBox3DRollbackSession::GetLastMispredictedMask() const { return Impl->Session.get_last_mispredicted_mask(); }
int64 UBox3DRollbackSession::GetTotalRollbackFrames() const { return Impl->Session.get_total_rollback_frames(); }
int64 UBox3DRollbackSession::GetTotalStalledTicks() const { return Impl->Session.get_total_stalled_ticks(); }
float UBox3DRollbackSession::GetFrameAdvantage() const { return (float)Impl->Session.get_frame_advantage(); }
int64 UBox3DRollbackSession::GetHashForFrame(int64 Frame) const { return Impl->Session.get_hash_for_frame(Frame); }
int32 UBox3DRollbackSession::GetIncompatiblePeerMask() const { return Impl->Session.get_incompatible_peer_mask(); }

int64 UBox3DRollbackSession::GetBuildFingerprint()
{
	return (int64)rb::RollbackSession::get_build_fingerprint();
}
