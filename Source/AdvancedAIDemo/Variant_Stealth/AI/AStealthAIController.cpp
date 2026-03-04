// Fill out your copyright notice in the Description page of Project Settings.


#include "AStealthAIController.h"

#include "AStealthGuardCharacter.h"
#include "PatrolRoute.h"
#include "StealthGuardState.h"
#include "VideoRecordingSystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

AAStealthAIController::AAStealthAIController()
{
	UE_LOG(LogTemp, Display, TEXT("Initializing"));
	// Create perception component
	AiPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SetPerceptionComponent(*AiPerceptionComponent);

	AiSightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("AISightConfig"));
	if (!AiSightConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create AISightConfig"));
	}
	else
	{
		// Configure sight sense
		AiSightConfig->SightRadius = 3500.0f;
		AiSightConfig->LoseSightRadius = 4000.0f;
		AiSightConfig->PeripheralVisionAngleDegrees = 90.0f;
		AiSightConfig->SetMaxAge(5.0f);
		AiSightConfig->AutoSuccessRangeFromLastSeenLocation = true;
		AiSightConfig->DetectionByAffiliation.bDetectEnemies = true;
		AiSightConfig->DetectionByAffiliation.bDetectNeutrals = false;
		AiSightConfig->DetectionByAffiliation.bDetectFriendlies = false;

		AiPerceptionComponent->ConfigureSense(*AiSightConfig);
	}


	//AiHearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("AIHearingConfig"));
	//if (!AiHearingConfig)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Failed to create AIHearingConfig"));
	//}
	//else
	//{
	//	// Configure hearing sense
	//	AiHearingConfig->HearingRange = 1500.0f;
	//	AiHearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	//	AiHearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	//	AiHearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

	//	AiPerceptionComponent->ConfigureSense(*AiHearingConfig);
	//}
	//AiPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAStealthAIController::OnTargetPerceptionUpdated);
}

bool AAStealthAIController::ReceiveAIAlert(const FAIAlertData& AlertData)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB || !GetWorld()) return false;

	// If already actively chasing with LOS, ignore weaker alerts
	const bool bHasLOS = BB->GetValueAsBool("HasLos");
	const bool bHasTarget = BB->GetValueAsObject("TargetActor") != nullptr;
	if (bHasTarget && bHasLOS && AlertData.Type != EAIAlertType::Confirmed)
	{
		return false;
	}

	// Always update investigate location for “go check it out”
	BB->SetValueAsVector("InvestigateLocation", AlertData.WorldLocation);

	// Raise suspicion using existing system (drives Investigate state naturally)
	const float CurrentSuspicion = BB->GetValueAsFloat("Suspicion");
	const float NewSuspicion = FMath::Clamp(FMath::Max(CurrentSuspicion, AlertData.Strength), 0.f, 1.f);
	BB->SetValueAsFloat("Suspicion", NewSuspicion);

	// If we know which actor (optional), set suspected actor
	if (AlertData.TargetActor.IsValid())
	{
		BB->SetValueAsObject("SuspectedActor", AlertData.TargetActor.Get());
	}

	// Hold alert state for a bit even if suspicion decays or they were never chasing.
	// This avoids fighting your UpdateGuardState logic.
	if (AlertData.Type == EAIAlertType::Confirmed)
	{
		const float Now = GetWorld()->GetTimeSeconds();
		BB->SetValueAsFloat("ExternalAlertUntil", Now + ExternalAlertHoldSeconds);

		// Also refresh LastSeenTime so your existing “recently seen” logic behaves nicely
		BB->SetValueAsFloat("LastSeenTime", Now);
	}

	return true;
}

void AAStealthAIController::BeginPlay()
{
	Super::BeginPlay();
#if WITH_GAMEPLAY_DEBUGGER
	UE_LOG(LogTemp, Warning, TEXT("Gameplay Debugger ENABLED"));
#else
	UE_LOG(LogTemp, Warning, TEXT("Gameplay Debugger DISABLED"));
#endif
}

void AAStealthAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	SetGenericTeamId(FGenericTeamId(1));

	if (!BehaviorTreeAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("BehaviorTreeAsset not set on %s"), *GetName());
		return;
	}

	UBlackboardComponent* BBComp = nullptr;

	// Initializes the blackboard using the BT's BlackboardAsset
	if (UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BBComp))
	{
		RunBehaviorTree(BehaviorTreeAsset);
		UE_LOG(LogTemp, Display, TEXT("Behavior tree started successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to initialize blackboard"));
	}

	if (AiPerceptionComponent)
	{
		AiPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
			this, &AAStealthAIController::OnTargetPerceptionUpdated);
	}
	UBlackboardComponent* BB = GetBlackboardComponent();
	AAStealthGuardCharacter* Guard = Cast<AAStealthGuardCharacter>(InPawn);
	if (!Guard) return;
	if (BB)
	{

		// Get Patrol Actor from blackboard
		int8 MaxPatrolIndex = 0;
		UE_LOG(LogTemp, Display, TEXT("Found PatrolRoute: %s"), Guard->PatrolRoute ? *Guard->PatrolRoute->GetName() : TEXT("None"));
		if (Guard->PatrolRoute)
		{
			MaxPatrolIndex = Guard->PatrolRoute->PointCount;
		}
		UE_LOG(LogTemp, Display, TEXT("MaxPatrolIndex: %d"), MaxPatrolIndex);
		BB->SetValueAsInt("PatrolIndex", FMath::RandRange(0, MaxPatrolIndex - 1));
	}
}

void AAStealthAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !GetBlackboardComponent())
	{
		return;
	}

	const bool bIsSight = (Stimulus.Type == UAISense::GetSenseID(UAISense_Sight::StaticClass()));
	const bool bIsHearing = (Stimulus.Type == UAISense::GetSenseID(UAISense_Hearing::StaticClass()));

	if (bIsSight)
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			UE_LOG(LogTemp, Display, TEXT("Saw Actor: %s"), *Actor->GetName());
			// Saw Target
			SetBBBool(Key_HasLos, true);
			SetBBObject(Key_SuspectedActor, Actor);
		}
		else
		{
			// Lost Sight
			SetBBBool(Key_HasLos, false);
			GetBlackboardComponent()->ClearValue(Key_TargetActor);
			SetBBVector(Key_InvestigateLocation, Stimulus.StimulusLocation);
		}
	}
	else if (bIsHearing)
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			// Heard Something
		}
	}
}

void AAStealthAIController::SetBBBool(const FName Key, bool bValue)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsBool(Key, bValue);
	}
}

void AAStealthAIController::SetBBVector(const FName Key, const FVector& Value)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsVector(Key, Value);
	}
}

void AAStealthAIController::SetBBObject(const FName Key, UObject* Value)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(Key, Value);
	}
}

void AAStealthAIController::SetBBFloat(const FName Key, float Value)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsFloat(Key, Value);
	}
}

void AAStealthAIController::UpdateGuardState()
{
	if (!Blackboard || !GetWorld()) return;

	const float Now = GetWorld()->GetTimeSeconds();

	const bool bHasTarget = Blackboard->GetValueAsObject(Key_TargetActor) != nullptr;
	const bool bHasLOS = Blackboard->GetValueAsBool(Key_HasLos);
	const float Suspicion = Blackboard->GetValueAsFloat(Key_Suspicion);
	const float ExternalAlertUntil = Blackboard->GetValueAsFloat(TEXT("ExternalAlertUntil"));
	const bool bExternalAlertActive = (Now <= ExternalAlertUntil);

	const uint8 CurrentStateRaw = Blackboard->GetValueAsEnum(TEXT("State"));
	const bool bWasChasing = (CurrentStateRaw == (uint8)EStealthGuardState::Chase);

	// Record "last time we had LOS"
	if (bHasTarget && bHasLOS)
	{
		Blackboard->SetValueAsFloat(TEXT("LastSeenTime"), Now);
	}

	const float LastSeenTime = Blackboard->GetValueAsFloat(TEXT("LastSeenTime"));
	const float TimeSinceSeen = Now - LastSeenTime;

	// Tunables
	const float InvestigateThreshold = 0.2f;
	const float AlertAfterChaseWindow = 5.0f; // seconds of alert/search after losing LOS

	EStealthGuardState NewState;

	// 1) Seeing target -> CHASE
	if (bHasTarget && bHasLOS)
	{
		NewState = EStealthGuardState::Chase;
	}
	// 2) External alert forces ALERT (unless chasing)
	else if (bExternalAlertActive)
	{
		NewState = EStealthGuardState::Alert;
	}
	// 3) Was chasing recently -> ALERT
	else if (bWasChasing && TimeSinceSeen <= AlertAfterChaseWindow)
	{
		NewState = EStealthGuardState::Alert;
	}
	// 4) Suspicion -> INVESTIGATE
	else if (Suspicion >= InvestigateThreshold)
	{
		NewState = EStealthGuardState::Investigate;
	}
	else
	{
		NewState = EStealthGuardState::Patrol;
	}

	// Optional: avoid redundant writes
	if ((uint8)NewState != CurrentStateRaw)
	{
		Blackboard->SetValueAsEnum(TEXT("State"), (uint8)NewState);
	}
}
