// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/AIAlertTypes.h"
#include "AICommunicationComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ADVANCEDAIDEMO_API UAICommunicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAICommunicationComponent();
	
	UFUNCTION(BlueprintCallable, Category="AI|Communication")
	void BroadcastAlert(EAIAlertType Type, const FVector& Location, float Strength = 1.0f);
	
	UFUNCTION(BlueprintCallable, Category="AI|Communication")
	void BroadCastAlertData(FAIAlertData& AlertData);
	
	UPROPERTY(EditAnywhere, Category="AI|Communication")
	float AlertRadius = 2000.f;
	
	UPROPERTY(EditAnywhere, Category="AI|Communication")
	float BroadcastCooldownSeconds = 1.0f;
	
	UPROPERTY(EditAnywhere, Category="AI|Communication")
	bool bSkipSelf;
	
	UPROPERTY(EditAnywhere, Category="AI|Communication")
	bool bRequireLineOfSightToReceiver = false;
	
	UPROPERTY(EditAnywhere, Category="AI|Communication")
	TEnumAsByte<ECollisionChannel> QueryChannel = ECC_Pawn;
	
private:
	double LastBroadcastTime = -DBL_MAX;
	
	bool CanBroadcastNow() const;
	bool HasLineOfSightTo(AActor* Other) const;
};
