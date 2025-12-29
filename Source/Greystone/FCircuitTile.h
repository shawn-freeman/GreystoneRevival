// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DataTypes.h"
#include "FCircuitTile.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct GREYSTONE_API FCircuitTile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) ECircuitTileType Type = ECircuitTileType::Empty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 RotationSteps = 0;   // 0..3
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bLocked = false;

	// runtime only; still ok to expose for UI highlighting
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bPowered = false;
};