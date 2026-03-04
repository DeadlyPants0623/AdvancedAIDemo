#include "BTTask_LookAround.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_LookAround::UBTTask_LookAround()
{
	NodeName = TEXT("Look Around (Turn Actor)");
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_LookAround::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	APawn* Pawn = AICon->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	if (bStopMovementOnStart)
	{
		AICon->StopMovement();
		AICon->ClearFocus(EAIFocusPriority::Gameplay);
	}

	StartYaw = NormalizeYaw(Pawn->GetActorRotation().Yaw);
	BuildYawTargets(StartYaw);

	if (TargetYaws.Num() < 2 || TotalDuration <= 0.f)
		return EBTNodeResult::Succeeded;

	StepIndex = 0;
	StepTime = TotalDuration / float(TargetYaws.Num());
	StepTimer = 0.f;

	return EBTNodeResult::InProgress;
}

void UBTTask_LookAround::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APawn* Pawn = AICon->GetPawn();
	if (!Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (StepIndex >= TargetYaws.Num())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// advance step timing
	StepTimer += DeltaSeconds;
	if (StepTimer >= StepTime)
	{
		StepTimer -= StepTime;
		StepIndex++;

		if (StepIndex >= TargetYaws.Num())
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}
	}

	// turn actor toward current target yaw
	const float TargetYaw = TargetYaws[StepIndex];
	const float CurrentYaw = NormalizeYaw(Pawn->GetActorRotation().Yaw);

	const float MaxDelta = TurnSpeedDegPerSec * DeltaSeconds;
	const float NewYaw = StepTowardYaw(CurrentYaw, TargetYaw, MaxDelta);

	FRotator NewRot = Pawn->GetActorRotation();
	NewRot.Yaw = NewYaw;
	Pawn->SetActorRotation(NewRot);
}

void UBTTask_LookAround::BuildYawTargets(float InStartYaw)
{
	TargetYaws.Reset();

	auto AddYaw = [this](float Yaw)
		{
			TargetYaws.Add(NormalizeYaw(Yaw));
		};

	Steps = FMath::Clamp(Steps, 2, 12);

	// Human-ish: start, +half, -half, +full, -full (or truncated/extended by Steps)
	TArray<float> Pattern;
	Pattern.Add(InStartYaw);
	Pattern.Add(InStartYaw + (YawRange * 0.5f));
	Pattern.Add(InStartYaw - (YawRange * 0.5f));
	Pattern.Add(InStartYaw + YawRange);
	Pattern.Add(InStartYaw - YawRange);

	for (int32 i = 0; i < Steps && i < Pattern.Num(); i++)
	{
		AddYaw(Pattern[i]);
	}

	// If Steps > 5, continue alternating further “micro scans”
	for (int32 i = 5; i < Steps; i++)
	{
		const float Alpha = float(i - 4) / float(FMath::Max(1, Steps - 4));
		const float Offset = YawRange * (0.25f + 0.75f * Alpha); // stays within range, but varied
		AddYaw(InStartYaw + ((i % 2) ? Offset : -Offset));
	}

	if (bReturnToStartYaw)
	{
		AddYaw(InStartYaw);
	}
}

float UBTTask_LookAround::NormalizeYaw(float YawDeg)
{
	return FRotator::NormalizeAxis(YawDeg);
}

float UBTTask_LookAround::StepTowardYaw(float CurrentYaw, float TargetYaw, float MaxDeltaYaw)
{
	// shortest signed delta [-180, 180]
	const float Delta = FRotator::NormalizeAxis(TargetYaw - CurrentYaw);
	const float Clamped = FMath::Clamp(Delta, -MaxDeltaYaw, MaxDeltaYaw);
	return NormalizeYaw(CurrentYaw + Clamped);
}