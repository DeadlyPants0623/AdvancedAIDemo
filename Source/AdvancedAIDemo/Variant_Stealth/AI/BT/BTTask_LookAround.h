// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_LookAround.generated.h"

UCLASS()
class ADVANCEDAIDEMO_API UBTTask_LookAround : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_LookAround();

protected:
	UPROPERTY(EditAnywhere, Category = "LookAround", meta = (ClampMin = "0.1"))
	float TotalDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "LookAround", meta = (ClampMin = "0.0"))
	float YawRange = 90.0f;

	UPROPERTY(EditAnywhere, Category = "LookAround", meta = (ClampMin = "2", ClampMax = "12"))
	int32 Steps = 5;

	// Degrees per second (how fast the actor can turn)
	UPROPERTY(EditAnywhere, Category = "LookAround", meta = (ClampMin = "10.0"))
	float TurnSpeedDegPerSec = 240.0f;

	UPROPERTY(EditAnywhere, Category = "LookAround")
	bool bReturnToStartYaw = true;

	UPROPERTY(EditAnywhere, Category = "LookAround")
	bool bStopMovementOnStart = true;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	float StartYaw = 0.f;
	TArray<float> TargetYaws;
	int32 StepIndex = 0;
	float StepTime = 0.f;
	float StepTimer = 0.f;

	void BuildYawTargets(float InStartYaw);
	static float NormalizeYaw(float YawDeg);
	static float StepTowardYaw(float CurrentYaw, float TargetYaw, float MaxDeltaYaw);
};