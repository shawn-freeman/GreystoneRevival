// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FCircuitPuzzleConfig.h"
#include "FCircuitTile.h"
#include "UCircuitPuzzle.generated.h"

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class GREYSTONE_API UCircuitPuzzle : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable) void StartNew(const FCircuitPuzzleConfig& InConfig);
	UFUNCTION(BlueprintCallable) bool RotateTile(int32 Index); // returns true if rotated
	UFUNCTION(BlueprintCallable) bool IsSolved() const;

	UFUNCTION(BlueprintCallable) const TArray<FCircuitTile>& GetTiles() const { return Tiles; }
	UFUNCTION(BlueprintCallable) int32 GetWidth() const { return Width; }
	UFUNCTION(BlueprintCallable) int32 GetHeight() const { return Height; }

	UFUNCTION(BlueprintCallable) void RecomputePower(); // BFS

	// Optional: events to drive UI
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleSolved);
	UPROPERTY(BlueprintAssignable) FOnPuzzleSolved OnSolved;

private:
	int32 Width = 0, Height = 0;
	FCircuitPuzzleConfig Config;
	TArray<FCircuitTile> Tiles;
	TArray<int32> SolutionRotations;

	// Helper functions
	void GenerateSolutionPath(int32 SourceIndex, int32 SinkIndex, FRandomStream& RandStream);
	void ScramblePuzzle(FRandomStream& RandStream);
	void ApplyLocks(FRandomStream& RandStream);

	bool AreAdjacent(int32 Index1, int32 Index2) const;
	void GetUnvisitedNeighbors(int32 Index, const TSet<int32>& Visited, TArray<int32>& OutNeighbors) const;
	int32 ChooseNextTile(int32 CurrentIndex, int32 TargetIndex, const TArray<int32>& Neighbors, FRandomStream& RandStream) const;

	ECircuitDir GetDirectionTo(int32 FromIndex, int32 ToIndex) const;
	int32 GetNeighborIndex(int32 Index, ECircuitDir Direction) const;
	ECircuitDir GetOppositeDirection(ECircuitDir Direction) const;
	TArray<ECircuitDir> GetTileConnections(int32 Index) const;
};
