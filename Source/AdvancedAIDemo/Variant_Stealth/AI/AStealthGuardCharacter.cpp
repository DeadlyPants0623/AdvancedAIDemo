// Fill out your copyright notice in the Description page of Project Settings.


#include "AStealthGuardCharacter.h"
#include "AStealthAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AAStealthGuardCharacter::AAStealthGuardCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AAStealthAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	HeadLamp = CreateDefaultSubobject<USpotLightComponent>(TEXT("HeadLamp"));
	HeadLamp->SetupAttachment(GetMesh(), TEXT("headLampSocket"));

	HeadLamp->Intensity = 20000.f;
	HeadLamp->bUseInverseSquaredFalloff = false;
	HeadLamp->AttenuationRadius = 1500.f;
	
	AIComms = CreateDefaultSubobject<UAICommunicationComponent>(TEXT("AIComms"));
}

// Called when the game starts or when spawned
void AAStealthGuardCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAStealthGuardCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAStealthGuardCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAStealthGuardCharacter::SetPatrolSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
}

void AAStealthGuardCharacter::SetInvestigateSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = InvestigateSpeed;
}

void AAStealthGuardCharacter::SetAlertSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = AlertSpeed;
}

void AAStealthGuardCharacter::SetChaseSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
}

void AAStealthGuardCharacter::SetDebugSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = DebugSpeed;
}
