// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataTypes.generated.h"

UENUM(BlueprintType)
enum class ECircuitDir : uint8 { Up, Right, Down, Left };

UENUM(BlueprintType)
enum class ECircuitTileType : uint8 { Empty, Source, Sink, Straight, Elbow, Tee, Cross, Blocker };
