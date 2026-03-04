// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_ConfirmTarget.generated.h"

/**
 * 
 */
UCLASS()
class ADVANCEDAIDEMO_API UBTService_ConfirmTarget : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_ConfirmTarget();
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
