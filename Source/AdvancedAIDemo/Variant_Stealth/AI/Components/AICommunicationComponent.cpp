// Fill out your copyright notice in the Description page of Project Settings.


#include "AICommunicationComponent.h"

#include "Core/AIAlertReceiver.h"
#include "Engine/OverlapResult.h"


// Sets default values for this component's properties
UAICommunicationComponent::UAICommunicationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAICommunicationComponent::BroadcastAlert(EAIAlertType Type, const FVector& Location, float Strength)
{
	FAIAlertData Alert;
	Alert.Type = Type;
	Alert.WorldLocation = Location;
	Alert.Sender = GetOwner();
	Alert.Strength = Strength;
	Alert.TimeStampSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	
	BroadCastAlertData(Alert);
}

void UAICommunicationComponent::BroadCastAlertData(FAIAlertData& AlertData)
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (!World || !Owner) return;
	if (!CanBroadcastNow()) return;

	LastBroadcastTime = World->GetTimeSeconds();

	TArray<FOverlapResult> Overlaps;
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(AlertRadius);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AICommsOverlap), false, Owner);
	Params.AddIgnoredActor(Owner);

	if (!World->OverlapMultiByChannel(
		Overlaps,
		Owner->GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		Sphere,
		Params))
	{
		return;
	}

	for (const FOverlapResult& R : Overlaps)
	{
		AActor* Other = R.GetActor();
		if (!Other || Other == Owner) continue;

		// We’ll receive on AIController, but overlap hits pawns.
		// If pawn has an AIController, forward alert to it via interface too.
		APawn* Pawn = Cast<APawn>(Other);
		AActor* ReceiverActor = Other;

		if (Pawn && Pawn->GetController())
		{
			ReceiverActor = Pawn->GetController();
		}

		if (ReceiverActor && ReceiverActor->GetClass()->ImplementsInterface(UAIAlertReceiver::StaticClass()))
		{
			IAIAlertReceiver* Receiver = Cast<IAIAlertReceiver>(ReceiverActor);
			if (Receiver)
			{
				Receiver->ReceiveAIAlert(AlertData);
			}
		}
	}
}

bool UAICommunicationComponent::CanBroadcastNow() const
{
	const UWorld* World = GetWorld();
	if (!World) return false;
	
	const double Now = World->GetTimeSeconds();
	return (Now - LastBroadcastTime) >= BroadcastCooldownSeconds;
}

bool UAICommunicationComponent::HasLineOfSightTo(AActor* Other) const
{
	if (!Other) return false;
	
	AActor* Owner = GetOwner();
	if (!Owner) return false;
	
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AICommsLOS), false, Owner);
	Params.AddIgnoredActor(Owner);
	Params.AddIgnoredActor(Other);
	
	const FVector Start = Owner->GetActorLocation();
	const FVector End = Other->GetActorLocation();
	
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	if (!bHit) return true;
	
	return Hit.GetActor() == Other;
}




