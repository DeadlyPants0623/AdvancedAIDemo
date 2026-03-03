#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SpotLightComponent.h"
#include "AStealthGuardCharacter.generated.h"

class AAStealthAIController;
class APatrolRoute;

UCLASS()
class ADVANCEDAIDEMO_API AAStealthGuardCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAStealthGuardCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI|Patrol")
	TObjectPtr<APatrolRoute> PatrolRoute;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Movement")
	float PatrolSpeed = 200.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI|Movement")
	float InvestigateSpeed = 300.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI|Movement")
	float AlertSpeed = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Movement")
	float ChaseSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Movement")
	float DebugSpeed = 2000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HeadLamp")
	TObjectPtr<USpotLightComponent> HeadLamp;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "AI|Movement")
	void SetPatrolSpeed();	
	
	UFUNCTION(BlueprintCallable, Category = "AI|Movement")
	void SetInvestigateSpeed();	
	
	UFUNCTION(BlueprintCallable, Category = "AI|Movement")
	void SetAlertSpeed();

	UFUNCTION(BlueprintCallable, Category = "AI|Movement")
	void SetChaseSpeed();

	UFUNCTION(BlueprintCallable, Category = "AI|Movement")
	void SetDebugSpeed();
};
