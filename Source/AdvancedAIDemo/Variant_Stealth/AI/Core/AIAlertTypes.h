#pragma once

#include "CoreMinimal.h"
#include "AIAlertTypes.generated.h"

UENUM(BlueprintType)
enum class EAIAlertType : uint8
{
	Suspicious UMETA(DisplayName = "Suspicious"),
	Confirmed UMETA(DisplayName = "Confirmed"),
	LostTarget UMETA(DisplayName = "Lost Target"),
	BodyFound UMETA(DisplayName = "Body Found In"),
};

USTRUCT(BlueprintType)
struct FAIAlertData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	EAIAlertType Type = EAIAlertType::Suspicious;
	
	UPROPERTY(BlueprintReadOnly)
	FVector WorldLocation = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Sender;
	
	UPROPERTY(BlueprintReadOnly)
	float Strength = 1.0f;
	
	UPROPERTY(BlueprintReadOnly)
	float TimeStampSeconds = 0.0f;
	
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> TargetActor;
};