#include "GameplayDebuggerCategory_Suspicion.h"

#include "AStealthAIController.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "AStealthGuardCharacter.h"
#include "AICommunicationComponent.h"
#include "StealthGuardState.h"

FGameplayDebuggerCategory_Suspicion::FGameplayDebuggerCategory_Suspicion()
{
	bShowOnlyWithDebugActor = true; // ONLY show for selected/focused debug actor
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_Suspicion::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_Suspicion());
}

void FGameplayDebuggerCategory_Suspicion::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	bValid = false;

	APawn* Pawn = Cast<APawn>(DebugActor);
	if (!Pawn) return;

	AAStealthAIController* MyAIC = Cast<AAStealthAIController>(Pawn->GetController());
	if (!MyAIC) return;

	UBlackboardComponent* BB = MyAIC->GetBlackboardComponent();
	if (!BB) return;

	PawnLocation = Pawn->GetActorLocation();

	// --- Blackboard state ---
	Suspicion = BB->GetValueAsFloat("Suspicion");
	StateRaw  = BB->GetValueAsEnum("State");

	bHasLOS   = BB->GetValueAsBool("HasLos");
	bHasTarget = (BB->GetValueAsObject("TargetActor") != nullptr);

	UObject* TargetObj = BB->GetValueAsObject("TargetActor");
	TargetName = TargetObj ? TargetObj->GetName() : TEXT("None");

	UObject* SuspectObj = BB->GetValueAsObject("SuspectedActor");
	SuspectName = SuspectObj ? SuspectObj->GetName() : TEXT("None");

	// --- External alert timing ---
	ExternalAlertUntil = BB->GetValueAsFloat("ExternalAlertUntil");
	const float Now = OwnerPC && OwnerPC->GetWorld() ? OwnerPC->GetWorld()->GetTimeSeconds() : 0.f;
	ExternalAlertRemaining = FMath::Max(0.f, ExternalAlertUntil - Now);

	// --- Controller tuning (single source of truth) ---
	SuspicionMaxGainDistance = MyAIC->SuspicionMaxGainDistance;
	ConfirmDistance          = MyAIC->ConfirmDistance;
	ConfirmSuspicion         = MyAIC->ConfirmSuspicion;

	// --- Comms component values ---
	CommsRadius = 0.f;
	CommsCooldown = 0.f;

	if (AAStealthGuardCharacter* Guard = Cast<AAStealthGuardCharacter>(Pawn))
	{
		if (Guard->AIComms)
		{
			CommsRadius = Guard->AIComms->AlertRadius;
			CommsCooldown = Guard->AIComms->BroadcastCooldownSeconds;
		}
	}

	bValid = true;
}
void FGameplayDebuggerCategory_Suspicion::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	if (!bValid || !OwnerPC) return;

	// Convert state enum to readable text
	const EStealthGuardState State = static_cast<EStealthGuardState>(StateRaw);

	CanvasContext.Printf(TEXT("{white}State: {yellow}%s"), *UEnum::GetValueAsString(State));
	CanvasContext.Printf(TEXT("{white}HasLOS: {yellow}%s    {white}HasTarget: {yellow}%s"),
		bHasLOS ? TEXT("true") : TEXT("false"),
		bHasTarget ? TEXT("true") : TEXT("false"));

	CanvasContext.Printf(TEXT("{white}Target: {yellow}%s    {white}Suspect: {yellow}%s"),
		*TargetName, *SuspectName);

	CanvasContext.Printf(TEXT("{white}Suspicion: {yellow}%.2f"), Suspicion);

	CanvasContext.Printf(TEXT("{yellow}Suspicion Range: %.0f"), SuspicionMaxGainDistance);
	CanvasContext.Printf(TEXT("{red}Confirm Range: %.0f  (ConfirmSuspicion: %.2f)"), ConfirmDistance, ConfirmSuspicion);

	// External alert info
	if (ExternalAlertRemaining > 0.f)
	{
		CanvasContext.Printf(TEXT("{cyan}ExternalAlert: ACTIVE  {white}(%.2fs remaining)"), ExternalAlertRemaining);
	}
	else
	{
		CanvasContext.Printf(TEXT("{cyan}ExternalAlert: inactive"));
	}

	// Comms info (only if component exists)
	if (CommsRadius > 0.f)
	{
		CanvasContext.Printf(TEXT("{green}Comms Radius: %.0f  {white}Cooldown: %.2fs"), CommsRadius, CommsCooldown);
	}

	UWorld* World = OwnerPC->GetWorld();
	if (!World) return;

	// Yellow suspicion ring
	DrawDebugCircle(
		World, PawnLocation, SuspicionMaxGainDistance, 64, FColor::Yellow,
		false, 0.f, 0, 3.f, FVector(1,0,0), FVector(0,1,0), false
	);

	// Red confirm ring
	DrawDebugCircle(
		World, PawnLocation, ConfirmDistance, 64, FColor::Red,
		false, 0.f, 0, 4.f, FVector(1,0,0), FVector(0,1,0), false
	);

	// Green comms broadcast ring
	if (CommsRadius > 0.f)
	{
		DrawDebugCircle(
			World, PawnLocation, CommsRadius, 64, FColor::Green,
			false, 0.f, 0, 2.f, FVector(1,0,0), FVector(0,1,0), false
		);
	}
}