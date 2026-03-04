// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolRoute.generated.h"

UCLASS()
class ADVANCEDAIDEMO_API APatrolRoute : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APatrolRoute();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	// How many points this route should have
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol")
	int32 PointCount = 5;

	// The point components (editable per instance)
	UPROPERTY(VisibleAnywhere, Instanced, Category = "Patrol")
	TArray<TObjectPtr<USceneComponent>> PatrolPoints;

	// Utility: return world locations for AI
	UFUNCTION(BlueprintCallable, Category = "Patrol")
	TArray<FVector> GetPatrolPointsLocations() const;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	void RebuildPoints();

};
