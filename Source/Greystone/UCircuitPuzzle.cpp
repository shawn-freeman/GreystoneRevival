// Fill out your copyright notice in the Description page of Project Settings.


#include "UCircuitPuzzle.h"
#include "Containers/Queue.h"
#include "Containers/Set.h"
#include "Math/RandomStream.h"

void UCircuitPuzzle::StartNew(const FCircuitPuzzleConfig& InConfig)
{
	Config = InConfig;
	Width = InConfig.Width;
	Height = InConfig.Height;

	// Initialize random stream
	FRandomStream RandStream;
	if (InConfig.Seed != 0)
	{
		RandStream.Initialize(InConfig.Seed);
	}
	else
	{
		RandStream.Initialize(FMath::Rand());
	}

	// Initialize grid - completely clear first
	const int32 TileCount = Width * Height;
	Tiles.Empty();
	SolutionRotations.Empty();
	Tiles.SetNum(TileCount);
	SolutionRotations.SetNum(TileCount);

	// Fill with empty tiles initially
	for (int32 i = 0; i < TileCount; ++i)
	{
		Tiles[i].Type = ECircuitTileType::Empty;
		Tiles[i].RotationSteps = 0;
		Tiles[i].bLocked = false;
		Tiles[i].bPowered = false;
		SolutionRotations[i] = 0;
	}

	// Place source and sink
	int32 SourceIndex = RandStream.RandRange(0, TileCount - 1);
	int32 SinkIndex = RandStream.RandRange(0, TileCount - 1);

	// Make sure source and sink are not the same and not adjacent
	while (SinkIndex == SourceIndex || AreAdjacent(SourceIndex, SinkIndex))
	{
		SinkIndex = RandStream.RandRange(0, TileCount - 1);
	}

	Tiles[SourceIndex].Type = ECircuitTileType::Source;
	Tiles[SinkIndex].Type = ECircuitTileType::Sink;

	// Generate a path from source to sink
	GenerateSolutionPath(SourceIndex, SinkIndex, RandStream);

	// Fill empty tiles with random pieces
	for (int32 i = 0; i < TileCount; ++i)
	{
		if (Tiles[i].Type == ECircuitTileType::Empty)
		{
			// Randomly choose tile type (favor simpler tiles)
			int32 Roll = RandStream.RandRange(0, 99);
			if (Roll < 40)
			{
				Tiles[i].Type = ECircuitTileType::Straight;
			}
			else if (Roll < 80)
			{
				Tiles[i].Type = ECircuitTileType::Elbow;
			}
			else if (Roll < 95)
			{
				Tiles[i].Type = ECircuitTileType::Tee;
			}
			else
			{
				Tiles[i].Type = ECircuitTileType::Cross;
			}

			// Random rotation
			Tiles[i].RotationSteps = RandStream.RandRange(0, 3);
		}

		// Store solution
		SolutionRotations[i] = Tiles[i].RotationSteps;
	}

	// Scramble puzzle (rotate tiles randomly)
	ScramblePuzzle(RandStream);

	// Apply locks to some tiles for difficulty
	ApplyLocks(RandStream);

	// Compute initial power state
	RecomputePower();
}

void UCircuitPuzzle::GenerateSolutionPath(int32 SourceIndex, int32 SinkIndex, FRandomStream& RandStream)
{
	// Simple path generation using random walk with backtracking
	TArray<int32> Path;
	Path.Add(SourceIndex);

	int32 CurrentIndex = SourceIndex;

	// Random walk toward sink
	while (CurrentIndex != SinkIndex)
	{
		// Build visited set from current path (so backtracking works correctly)
		TSet<int32> Visited;
		for (int32 PathIndex : Path)
		{
			Visited.Add(PathIndex);
		}

		TArray<int32> Neighbors;
		GetUnvisitedNeighbors(CurrentIndex, Visited, Neighbors);

		if (Neighbors.Num() == 0)
		{
			// Backtrack
			if (Path.Num() > 1)
			{
				Path.Pop(); // Remove last tile from path (this also removes it from Visited next iteration)
				CurrentIndex = Path.Last();
			}
			else
			{
				// Reset if we get stuck at source
				Path.Empty();
				Path.Add(SourceIndex);
				CurrentIndex = SourceIndex;
			}
			continue;
		}

		// Prefer moving toward sink
		int32 NextIndex = ChooseNextTile(CurrentIndex, SinkIndex, Neighbors, RandStream);
		Path.Add(NextIndex);
		CurrentIndex = NextIndex;
	}

	// Validate path
	if (Path.Num() < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("Path too short: %d"), Path.Num());
		return;
	}

	if (Path[0] != SourceIndex)
	{
		UE_LOG(LogTemp, Error, TEXT("Path doesn't start at Source! Path[0]=%d, SourceIndex=%d"), Path[0], SourceIndex);
		return;
	}

	if (Path[Path.Num() - 1] != SinkIndex)
	{
		UE_LOG(LogTemp, Error, TEXT("Path doesn't end at Sink! Path[Last]=%d, SinkIndex=%d"), Path[Path.Num() - 1], SinkIndex);
		return;
	}

	// Remove any duplicate indices from path (shouldn't happen, but safety check)
	TArray<int32> CleanPath;
	TSet<int32> PathSet;
	for (int32 Idx : Path)
	{
		if (!PathSet.Contains(Idx))
		{
			CleanPath.Add(Idx);
			PathSet.Add(Idx);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Duplicate tile in path: %d"), Idx);
		}
	}

	if (CleanPath.Num() != Path.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Removed %d duplicate tiles from path"), Path.Num() - CleanPath.Num());
	}

	Path = CleanPath;

	// Convert path to tiles
	for (int32 i = 0; i < Path.Num(); ++i)
	{
		int32 Idx = Path[i];

		// Verify this tile hasn't been set yet (except for source/sink)
		if (i != 0 && i != Path.Num() - 1)
		{
			// Middle tiles should be Empty before we set them
			if (Tiles[Idx].Type != ECircuitTileType::Empty)
			{
				// This tile was already set somehow - skip it
				continue;
			}
		}

		if (i == 0)
		{
			// Source - determine rotation based on next tile
			if (Path.Num() > 1)
			{
				int32 NextIdx = Path[i + 1];
				ECircuitDir DirToNext = GetDirectionTo(Idx, NextIdx);
				Tiles[Idx].RotationSteps = static_cast<int32>(DirToNext);
			}
		}
		else if (i == Path.Num() - 1)
		{
			// Sink - determine rotation based on previous tile
			if (Path.Num() > 1)
			{
				int32 PrevIdx = Path[i - 1];
				ECircuitDir DirFromPrev = GetDirectionTo(Idx, PrevIdx);
				Tiles[Idx].RotationSteps = static_cast<int32>(DirFromPrev);
			}
		}
		else
		{
			// Middle tiles - determine type and rotation
			int32 PrevIdx = Path[i - 1];
			int32 NextIdx = Path[i + 1];

			ECircuitDir DirFromPrev = GetDirectionTo(Idx, PrevIdx);
			ECircuitDir DirToNext = GetDirectionTo(Idx, NextIdx);

			int32 Dir1 = static_cast<int32>(DirFromPrev);
			int32 Dir2 = static_cast<int32>(DirToNext);

			// Check if it's a straight or elbow
			int32 DirDiff = FMath::Abs(Dir1 - Dir2);

			if (DirDiff == 2) // Opposite directions = straight
			{
				Tiles[Idx].Type = ECircuitTileType::Straight;
				// For straight: 0 = vertical (Up/Down), 1 = horizontal (Left/Right)
				Tiles[Idx].RotationSteps = (Dir1 == 0 || Dir1 == 2) ? 0 : 1;
			}
			else // Perpendicular = elbow
			{
				Tiles[Idx].Type = ECircuitTileType::Elbow;
				// Elbow rotation R connects directions R and (R+1)%4
				// Find which rotation connects Dir1 and Dir2
				if (Dir2 == (Dir1 + 1) % 4)
				{
					Tiles[Idx].RotationSteps = Dir1;
				}
				else if (Dir1 == (Dir2 + 1) % 4)
				{
					Tiles[Idx].RotationSteps = Dir2;
				}
				else
				{
					// Fallback (shouldn't happen with perpendicular dirs)
					Tiles[Idx].RotationSteps = FMath::Min(Dir1, Dir2);
				}
			}
		}
	}
}

void UCircuitPuzzle::ScramblePuzzle(FRandomStream& RandStream)
{
	int32 ScrambleCount = FMath::Max(Config.MinScramble, Width * Height / 2);

	for (int32 i = 0; i < ScrambleCount; ++i)
	{
		int32 RandomIndex = RandStream.RandRange(0, Tiles.Num() - 1);

		// Don't scramble source/sink if not allowed
		if (Tiles[RandomIndex].Type == ECircuitTileType::Source && !Config.bAllowRotateSource)
			continue;
		if (Tiles[RandomIndex].Type == ECircuitTileType::Sink && !Config.bAllowRotateSink)
			continue;

		// Rotate by random amount
		int32 RotateAmount = RandStream.RandRange(1, 3);
		Tiles[RandomIndex].RotationSteps = (Tiles[RandomIndex].RotationSteps + RotateAmount) % 4;
	}
}

void UCircuitPuzzle::ApplyLocks(FRandomStream& RandStream)
{
	int32 LocksToApply = FMath::Min(Config.LockCount, Tiles.Num());

	for (int32 i = 0; i < LocksToApply; ++i)
	{
		int32 RandomIndex = RandStream.RandRange(0, Tiles.Num() - 1);

		// Don't lock source or sink
		if (Tiles[RandomIndex].Type == ECircuitTileType::Source ||
			Tiles[RandomIndex].Type == ECircuitTileType::Sink)
		{
			continue;
		}

		Tiles[RandomIndex].bLocked = true;
	}
}

bool UCircuitPuzzle::RotateTile(int32 Index)
{
	if (Index < 0 || Index >= Tiles.Num())
		return false;

	if (Tiles[Index].bLocked)
		return false;

	if (Tiles[Index].Type == ECircuitTileType::Source && !Config.bAllowRotateSource)
		return false;

	if (Tiles[Index].Type == ECircuitTileType::Sink && !Config.bAllowRotateSink)
		return false;

	Tiles[Index].RotationSteps = (Tiles[Index].RotationSteps + 1) % 4;
	RecomputePower();

	if (IsSolved())
	{
		OnSolved.Broadcast();
	}

	return true;
}

bool UCircuitPuzzle::IsSolved() const
{
	// Find source and check if sink is powered
	for (int32 i = 0; i < Tiles.Num(); ++i)
	{
		if (Tiles[i].Type == ECircuitTileType::Sink && Tiles[i].bPowered)
		{
			return true;
		}
	}
	return false;
}

void UCircuitPuzzle::RecomputePower()
{
	// Reset all power
	for (FCircuitTile& Tile : Tiles)
	{
		Tile.bPowered = false;
	}

	// Find source
	int32 SourceIndex = -1;
	for (int32 i = 0; i < Tiles.Num(); ++i)
	{
		if (Tiles[i].Type == ECircuitTileType::Source)
		{
			SourceIndex = i;
			break;
		}
	}

	if (SourceIndex == -1)
		return;

	// BFS from source
	TQueue<int32> Queue;
	Queue.Enqueue(SourceIndex);
	Tiles[SourceIndex].bPowered = true;

	while (!Queue.IsEmpty())
	{
		int32 CurrentIndex;
		Queue.Dequeue(CurrentIndex);

		// Get connections from current tile
		TArray<ECircuitDir> Connections = GetTileConnections(CurrentIndex);

		for (ECircuitDir Dir : Connections)
		{
			int32 NeighborIndex = GetNeighborIndex(CurrentIndex, Dir);
			if (NeighborIndex == -1)
				continue;

			if (Tiles[NeighborIndex].bPowered)
				continue;

			// Check if neighbor connects back
			ECircuitDir OppositeDir = GetOppositeDirection(Dir);
			TArray<ECircuitDir> NeighborConnections = GetTileConnections(NeighborIndex);

			if (NeighborConnections.Contains(OppositeDir))
			{
				Tiles[NeighborIndex].bPowered = true;
				Queue.Enqueue(NeighborIndex);
			}
		}
	}
}

// Helper functions
bool UCircuitPuzzle::AreAdjacent(int32 Index1, int32 Index2) const
{
	int32 X1 = Index1 % Width;
	int32 Y1 = Index1 / Width;
	int32 X2 = Index2 % Width;
	int32 Y2 = Index2 / Width;

	int32 DX = FMath::Abs(X1 - X2);
	int32 DY = FMath::Abs(Y1 - Y2);

	return (DX == 1 && DY == 0) || (DX == 0 && DY == 1);
}

void UCircuitPuzzle::GetUnvisitedNeighbors(int32 Index, const TSet<int32>& Visited, TArray<int32>& OutNeighbors) const
{
	OutNeighbors.Empty();

	int32 X = Index % Width;
	int32 Y = Index / Width;

	// Up
	if (Y > 0)
	{
		int32 Neighbor = (Y - 1) * Width + X;
		if (!Visited.Contains(Neighbor))
			OutNeighbors.Add(Neighbor);
	}

	// Right
	if (X < Width - 1)
	{
		int32 Neighbor = Y * Width + (X + 1);
		if (!Visited.Contains(Neighbor))
			OutNeighbors.Add(Neighbor);
	}

	// Down
	if (Y < Height - 1)
	{
		int32 Neighbor = (Y + 1) * Width + X;
		if (!Visited.Contains(Neighbor))
			OutNeighbors.Add(Neighbor);
	}

	// Left
	if (X > 0)
	{
		int32 Neighbor = Y * Width + (X - 1);
		if (!Visited.Contains(Neighbor))
			OutNeighbors.Add(Neighbor);
	}
}

int32 UCircuitPuzzle::ChooseNextTile(int32 CurrentIndex, int32 TargetIndex, const TArray<int32>& Neighbors, FRandomStream& RandStream) const
{
	int32 CurrentX = CurrentIndex % Width;
	int32 CurrentY = CurrentIndex / Width;
	int32 TargetX = TargetIndex % Width;
	int32 TargetY = TargetIndex / Width;

	// 70% chance to move toward target, 30% random
	if (RandStream.FRand() < 0.7f)
	{
		int32 BestNeighbor = Neighbors[0];
		float BestDist = FLT_MAX;

		for (int32 Neighbor : Neighbors)
		{
			int32 NX = Neighbor % Width;
			int32 NY = Neighbor / Width;
			float Dist = FMath::Sqrt(static_cast<float>(FMath::Square(TargetX - NX) + FMath::Square(TargetY - NY)));

			if (Dist < BestDist)
			{
				BestDist = Dist;
				BestNeighbor = Neighbor;
			}
		}

		return BestNeighbor;
	}
	else
	{
		return Neighbors[RandStream.RandRange(0, Neighbors.Num() - 1)];
	}
}

ECircuitDir UCircuitPuzzle::GetDirectionTo(int32 FromIndex, int32 ToIndex) const
{
	int32 FromX = FromIndex % Width;
	int32 FromY = FromIndex / Width;
	int32 ToX = ToIndex % Width;
	int32 ToY = ToIndex / Width;

	if (ToY < FromY) return ECircuitDir::Up;
	if (ToX > FromX) return ECircuitDir::Right;
	if (ToY > FromY) return ECircuitDir::Down;
	return ECircuitDir::Left;
}

int32 UCircuitPuzzle::GetNeighborIndex(int32 Index, ECircuitDir Direction) const
{
	int32 X = Index % Width;
	int32 Y = Index / Width;

	switch (Direction)
	{
		case ECircuitDir::Up:
			if (Y > 0) return (Y - 1) * Width + X;
			break;
		case ECircuitDir::Right:
			if (X < Width - 1) return Y * Width + (X + 1);
			break;
		case ECircuitDir::Down:
			if (Y < Height - 1) return (Y + 1) * Width + X;
			break;
		case ECircuitDir::Left:
			if (X > 0) return Y * Width + (X - 1);
			break;
	}

	return -1;
}

ECircuitDir UCircuitPuzzle::GetOppositeDirection(ECircuitDir Direction) const
{
	switch (Direction)
	{
		case ECircuitDir::Up: return ECircuitDir::Down;
		case ECircuitDir::Right: return ECircuitDir::Left;
		case ECircuitDir::Down: return ECircuitDir::Up;
		case ECircuitDir::Left: return ECircuitDir::Right;
	}
	return ECircuitDir::Up;
}

TArray<ECircuitDir> UCircuitPuzzle::GetTileConnections(int32 Index) const
{
	TArray<ECircuitDir> Connections;

	const FCircuitTile& Tile = Tiles[Index];
	int32 Rot = Tile.RotationSteps;

	switch (Tile.Type)
	{
		case ECircuitTileType::Source:
		case ECircuitTileType::Sink:
			// Single connection in rotation direction
			Connections.Add(static_cast<ECircuitDir>(Rot % 4));
			break;

		case ECircuitTileType::Straight:
			// Two opposite connections
			Connections.Add(static_cast<ECircuitDir>(Rot % 2 == 0 ? 0 : 1)); // Up or Right
			Connections.Add(static_cast<ECircuitDir>(Rot % 2 == 0 ? 2 : 3)); // Down or Left
			break;

		case ECircuitTileType::Elbow:
			// Two perpendicular connections
			Connections.Add(static_cast<ECircuitDir>((0 + Rot) % 4));
			Connections.Add(static_cast<ECircuitDir>((1 + Rot) % 4));
			break;

		case ECircuitTileType::Tee:
			// Three connections
			Connections.Add(static_cast<ECircuitDir>((0 + Rot) % 4));
			Connections.Add(static_cast<ECircuitDir>((1 + Rot) % 4));
			Connections.Add(static_cast<ECircuitDir>((3 + Rot) % 4));
			break;

		case ECircuitTileType::Cross:
			// All four connections
			Connections.Add(ECircuitDir::Up);
			Connections.Add(ECircuitDir::Right);
			Connections.Add(ECircuitDir::Down);
			Connections.Add(ECircuitDir::Left);
			break;

		case ECircuitTileType::Empty:
		case ECircuitTileType::Blocker:
		default:
			break;
	}

	return Connections;
}
