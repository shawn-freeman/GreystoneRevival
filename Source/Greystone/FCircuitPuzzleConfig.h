// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FCircuitPuzzleConfig.generated.h"

USTRUCT(BlueprintType)
struct GREYSTONE_API FCircuitPuzzleConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Width = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Height = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float TimeLimitSeconds = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxMoves = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAllowRotateSource = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAllowRotateSink = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 LockCount = 0;       // difficulty knob
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MinScramble = 6;     // difficulty knob
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Seed = 0;           // 0 = random seed
};