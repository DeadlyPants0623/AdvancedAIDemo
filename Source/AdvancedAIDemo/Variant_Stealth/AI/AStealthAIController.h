// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "AIAlertReceiver.h"
#include "AIAlertTypes.h"
#include "AStealthAIController.generated.h"

class UBehaviorTree;
class UBlackboardComponent;
class APatrolRoute;

UCLASS()
class ADVANCEDAIDEMO_API AAStealthAIController : public AAIController, public IAIAlertReceiver
{
	GENERATED_BODY()

public:
	AAStealthAIController();
	
	virtual bool ReceiveAIAlert(const FAIAlertData& AlertData) override;
	
	UPROPERTY(EditAnywhere, Category = "AI|Communication")
	float ExternalAlertHoldSeconds = 4.0f;

protected:

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UAIPerceptionComponent* AiPerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAISenseConfig_Sight* AiSightConfig;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	//UAISenseConfig_Hearing* AiHearingConfig;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	UBehaviorTree* BehaviorTreeAsset;
	
private:
	void SetBBBool(const FName Key, bool bValue);
	void SetBBVector(const FName Key, const FVector& Value);
	void SetBBObject(const FName Key, UObject* Value);
	void SetBBFloat(const FName Key, float Value);
	
	
	// Blackboard Keys
	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard Keys")
	FName Key_TargetActor = "TargetActor";
	
	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard Keys")
	FName Key_SuspectedActor = "SuspectedActor";
	
	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard Keys")
	FName Key_InvestigateLocation = "InvestigateLocation";
	
	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard Keys")
	FName Key_HasLos = "HasLos";
	
	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard Keys")
	FName Key_Suspicion = "Suspicion";
	
	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard Keys")
	FName Key_InvestigateActive = "InvestigateActive";
	
	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard Keys")
	FName Key_PatrolActor = "PatrolActor";
	
public:
	// ---- Suspicion tuning (single source of truth) ----
	UPROPERTY(EditDefaultsOnly, Category="AI|Suspicion")
	float SuspicionMaxGainDistance = 2500.f;

	UPROPERTY(EditDefaultsOnly, Category="AI|Suspicion")
	float SuspicionMinDot = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category="AI|Suspicion")
	float SuspicionBaseGainPerSecond = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category="AI|Suspicion")
	float SuspicionDecayPerSecond = 0.25f;

	// ---- Confirm tuning ----
	UPROPERTY(EditDefaultsOnly, Category="AI|Confirm")
	float ConfirmDistance = 600.f;

	UPROPERTY(EditDefaultsOnly, Category="AI|Confirm")
	float ConfirmSuspicion = 0.95f;
	
	UPROPERTY(EditDefaultsOnly, Category="AI|Patrol")
	int8 StartingPatrolIndex = 0;
	
	UFUNCTION(BlueprintCallable, Category="AI|State")
	void UpdateGuardState();
	
};
