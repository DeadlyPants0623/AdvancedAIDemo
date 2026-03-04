// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_Suspicion.generated.h"

/**
 * 
 */
UCLASS()
class ADVANCEDAIDEMO_API UBTService_Suspicion : public UBTService
{
	GENERATED_BODY()

public:
	
	UBTService_Suspicion();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
