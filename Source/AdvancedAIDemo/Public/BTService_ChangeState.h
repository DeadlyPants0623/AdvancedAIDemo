// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_ChangeState.generated.h"

/**
 * 
 */
UCLASS()
class ADVANCEDAIDEMO_API UBTService_ChangeState : public UBTService
{
	GENERATED_BODY()
	
	UBTService_ChangeState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
