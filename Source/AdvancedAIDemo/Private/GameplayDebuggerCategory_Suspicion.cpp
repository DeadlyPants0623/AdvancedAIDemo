#include "GameplayDebuggerCategory_Suspicion.h"

#include "AStealthAIController.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "GameplayDebugger.h"

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

	// Read live runtime state
	Suspicion = BB->GetValueAsFloat("Suspicion");
	PawnLocation = Pawn->GetActorLocation();

	// Read tuning from ONE place (AIController)
	SuspicionMaxGainDistance = MyAIC->SuspicionMaxGainDistance;
	ConfirmDistance          = MyAIC->ConfirmDistance;
	ConfirmSuspicion         = MyAIC->ConfirmSuspicion;

	bValid = true;
}

void FGameplayDebuggerCategory_Suspicion::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	if (!bValid || !OwnerPC) return;

	CanvasContext.Printf(TEXT("{yellow}Suspicion Range: %.0f"), SuspicionMaxGainDistance);
	CanvasContext.Printf(TEXT("{red}Confirm Range: %.0f  (ConfirmSuspicion: %.2f)"), ConfirmDistance, ConfirmSuspicion);
	CanvasContext.Printf(TEXT("Suspicion: %.2f"), Suspicion);

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
}