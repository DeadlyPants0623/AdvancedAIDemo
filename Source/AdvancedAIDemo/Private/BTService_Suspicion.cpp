// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_Suspicion.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "AIController.h"
#include "AStealthAIController.h"
#include "StealthGuardState.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_Suspicion::UBTService_Suspicion()
{
	bNotifyTick = true;
	Interval = 0.15f;
}

void UBTService_Suspicion::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AIC = OwnerComp.GetAIOwner();
	AAStealthAIController* MyAIC = Cast<AAStealthAIController>(AIC);
	if (!MyAIC) return;

	const float MaxGainDistance = MyAIC->SuspicionMaxGainDistance;
	const float MinDotForSuspicion = MyAIC->SuspicionMinDot;
	const float BaseGainPerSecond = MyAIC->SuspicionBaseGainPerSecond;
	const float DecayPerSecond = MyAIC->SuspicionDecayPerSecond;
	if (!BB || !AIC) return;

	APawn* Pawn = AIC->GetPawn();
	if (!Pawn) return;

	const bool bHasLOS = BB->GetValueAsBool("HasLos");
	AActor* Suspect = Cast<AActor>(BB->GetValueAsObject("SuspectedActor"));

	float Suspicion = BB->GetValueAsFloat("Suspicion");

	if (bHasLOS && Suspect)
	{
		const FVector ToTarget = Suspect->GetActorLocation() - Pawn->GetActorLocation();
		const float Dist = ToTarget.Size();

		// Distance factor (near=1, far=0)
		const float Dist01 = FMath::Clamp(1.f - (Dist / MaxGainDistance), 0.f, 1.f);

		// Angle factor (center=1, edge=0)
		const FVector Forward = Pawn->GetActorForwardVector().GetSafeNormal();
		const FVector Dir = ToTarget.GetSafeNormal();
		const float Dot = FVector::DotProduct(Forward, Dir);
		const float Angle01 = FMath::Clamp((Dot - MinDotForSuspicion) / (1.f - MinDotForSuspicion), 0.f, 1.f);

		Suspicion = FMath::Clamp(Suspicion + (BaseGainPerSecond * Dist01 * Angle01) * DeltaSeconds, 0.f, 1.f);

		// Keep pushing investigate toward what we “think” we see
		BB->SetValueAsVector("InvestigateLocation", Suspect->GetActorLocation());
	}
	else
	{
		if (BB->GetValueAsEnum("State") != uint8(EStealthGuardState::Investigate))
		{
			// If we lost sight while investigating, keep suspicion up a bit longer
			Suspicion = FMath::Clamp(Suspicion - (DecayPerSecond * 0.5f) * DeltaSeconds, 0.f, 1.f);
		}
		else
		{
			Suspicion = FMath::Clamp(Suspicion - DecayPerSecond * DeltaSeconds, 0.f, 1.f);
		}

		// Optional: clear suspect when fully calm
		if (Suspicion <= 0.01f)
		{
			BB->SetValueAsObject("SuspectedActor", nullptr);
		}
	}

	BB->SetValueAsFloat("Suspicion", Suspicion);
}
