// Fill out your copyright notice in the Description page of Project Settings.


#include "AStealthAIController.h"

#include "StealthGuardState.h"
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
	if (UBlackboardComponent * BB = GetBlackboardComponent())
	{
		BB->SetValueAsBool(Key, bValue);
	}
		
}

void AAStealthAIController::SetBBVector(const FName Key, const FVector& Value)
{
	if (UBlackboardComponent * BB = GetBlackboardComponent())
	{
		BB->SetValueAsVector(Key, Value);
	}
}

void AAStealthAIController::SetBBObject(const FName Key, UObject* Value)
{
	if (UBlackboardComponent * BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(Key, Value);
	}
}

void AAStealthAIController::SetBBFloat(const FName Key, float Value)
{
	if (UBlackboardComponent * BB = GetBlackboardComponent())
	{
		BB->SetValueAsFloat(Key, Value);
	}
}

void AAStealthAIController::UpdateGuardState()
{
	if (!Blackboard || !GetWorld()) return;

	const float Now = GetWorld()->GetTimeSeconds();

	const bool bHasTarget = Blackboard->GetValueAsObject(Key_TargetActor) != nullptr;
	const bool bHasLOS    = Blackboard->GetValueAsBool(Key_HasLos);
	const float Suspicion = Blackboard->GetValueAsFloat(Key_Suspicion);

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

	// 1) Highest priority: currently seeing target -> CHASE
	if (bHasTarget && bHasLOS)
	{
		NewState = EStealthGuardState::Chase;
	}
	// 2) If we were chasing and just lost LOS recently -> ALERT
	else if (bWasChasing && TimeSinceSeen <= AlertAfterChaseWindow)
	{
		NewState = EStealthGuardState::Alert;
	}
	// 3) Suspicion thresholds -> INVESTIGATE (or ALERT if you want a high-suspicion alert)
	else if (Suspicion >= InvestigateThreshold)
	{
		NewState = EStealthGuardState::Investigate;
	}
	// 4) Otherwise -> PATROL
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
