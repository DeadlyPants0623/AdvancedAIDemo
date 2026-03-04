// Fill out your copyright notice in the Description page of Project Settings.


#include "PatrolRoute.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "EngineUtils.h"

static TAutoConsoleVariable<int32> CVarPatrolDebugShow(
	TEXT("patrolpoints.debug.show"),
	0,
	TEXT("Show patrol route debug.\n0 = Off\n1 = On"),
	ECVF_Default);

// Sets default values
APatrolRoute::APatrolRoute()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void APatrolRoute::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Never build on the CDO
	if (HasAnyFlags(RF_ClassDefaultObject)) return;

	RebuildPoints();
}

void APatrolRoute::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CVarPatrolDebugShow.GetValueOnGameThread() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	const float SphereSize = 25.f;
	const FColor Color = FColor::Green;

	for (int32 i = 0; i < PatrolPoints.Num(); i++)
	{
		if (!PatrolPoints[i]) continue;

		FVector Location = PatrolPoints[i]->GetComponentLocation();

		// Draw sphere
		DrawDebugSphere(
			World,
			Location,
			SphereSize,
			12,
			Color,
			false,
			-1.f,
			0,
			2.f
		);

		// Draw line to next point
		if (i < PatrolPoints.Num() - 1 && PatrolPoints[i + 1])
		{
			DrawDebugLine(
				World,
				Location,
				PatrolPoints[i + 1]->GetComponentLocation(),
				FColor::Yellow,
				false,
				-1.f,
				0,
				2.f
			);
		}
	}
}

#if WITH_EDITOR
void APatrolRoute::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropName = PropertyChangedEvent.GetPropertyName();
	if (PropName == GET_MEMBER_NAME_CHECKED(APatrolRoute, PointCount))
	{
		PointCount = FMath::Max(0, PointCount);
		RebuildPoints();
	}
}
#endif

TArray<FVector> APatrolRoute::GetPatrolPointsLocations() const
{
	TArray<FVector> Out;
	Out.Reserve(PatrolPoints.Num());

	for (const USceneComponent* P : PatrolPoints)
	{
		if (P)
		{
			Out.Add(P->GetComponentLocation());
		}
	}
	return Out;
}

void APatrolRoute::RebuildPoints()
{
	if (HasAnyFlags(RF_ClassDefaultObject)) return;

	// Remove extras
	while (PatrolPoints.Num() > PointCount)
	{
		if (USceneComponent* Comp = PatrolPoints.Pop())
		{
			Comp->DestroyComponent();
		}
	}

	// Add missing
	while (PatrolPoints.Num() < PointCount)
	{
		const int32 Index = PatrolPoints.Num();
		const FName Name = *FString::Printf(TEXT("PatrolPoint_%d"), Index);

		USceneComponent* NewPoint = NewObject<USceneComponent>(this, USceneComponent::StaticClass(), Name, RF_Transactional);
		NewPoint->SetupAttachment(Root);

		AddInstanceComponent(NewPoint);          // important for editor ownership/serialization rules
		NewPoint->RegisterComponent();

		NewPoint->SetRelativeLocation(FVector(200.f * Index, 0.f, 0.f));

		PatrolPoints.Add(NewPoint);
	}
}
