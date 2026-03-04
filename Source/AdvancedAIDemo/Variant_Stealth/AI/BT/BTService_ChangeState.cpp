// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_ChangeState.h"

#include "AStealthAIController.h"

UBTService_ChangeState::UBTService_ChangeState()
{
	bNotifyTick = true;
	Interval = 0.15f;
}

void UBTService_ChangeState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	AAIController* AIC = OwnerComp.GetAIOwner();
	AAStealthAIController* MyAIC = Cast<AAStealthAIController>(AIC);
	
	MyAIC->UpdateGuardState();
}
