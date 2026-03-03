// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_ConfirmTarget.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AStealthAIController.h"
#include "StealthGuardState.h"

UBTService_ConfirmTarget::UBTService_ConfirmTarget()
{
	bNotifyTick = true;
	Interval = 0.15f;
}

void UBTService_ConfirmTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AIC = OwnerComp.GetAIOwner();
	AAStealthAIController* MyAIC = Cast<AAStealthAIController>(AIC);
	
	if (!MyAIC) return;
	
	const float ConfirmDistance = MyAIC->ConfirmDistance;
	const float ConfirmSuspicion = MyAIC->ConfirmSuspicion;
	
	if (!BB || !AIC) return;

	APawn* Pawn = AIC->GetPawn();
	if (!Pawn) return;

	// Already confirmed
	if (BB->GetValueAsObject("TargetActor") != nullptr) return;
	
	if (!BB->GetValueAsBool("HasLos")) return;

	AActor* Suspect = Cast<AActor>(BB->GetValueAsObject("SuspectedActor"));
	if (!Suspect) return;

	const float Suspicion = BB->GetValueAsFloat("Suspicion");
	const float Dist = FVector::Dist(Pawn->GetActorLocation(), Suspect->GetActorLocation());

	if (Dist <= ConfirmDistance || Suspicion >= ConfirmSuspicion)
	{
		BB->SetValueAsFloat("Suspicion", 1.0f);
		BB->SetValueAsObject("TargetActor", Suspect);
	}
}
