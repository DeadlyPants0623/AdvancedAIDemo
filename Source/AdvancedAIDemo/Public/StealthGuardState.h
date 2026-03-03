// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StealthGuardState.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EStealthGuardState : uint8
{
	Null UMETA(DisplayName = "NULL"),
	Patrol UMETA(DisplayName = "Patrol"),
	Investigate UMETA(DisplayName = "Investigate"),
	Alert UMETA(DisplayName = "Alert"),
	Chase UMETA(DisplayName = "Chase"),
	Debug UMETA(DisplayName = "Debug")
};
