#pragma once

#include "CoreMinimal.h"
#include "GameplayDebuggerCategory.h"

class FGameplayDebuggerCategory_Suspicion : public FGameplayDebuggerCategory
{
public:
	FGameplayDebuggerCategory_Suspicion();

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

private:
	bool bValid = false;

	FVector PawnLocation = FVector::ZeroVector;
	float Suspicion = 0.f;

	float SuspicionMaxGainDistance = 0.f;
	float ConfirmDistance = 0.f;
	float ConfirmSuspicion = 0.f;
};