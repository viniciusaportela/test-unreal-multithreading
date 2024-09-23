// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constants/GameConstants.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChunkHelper.generated.h"

/**
 * 
 */
UCLASS()
class MULTITHREADTEST_API UChunkHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static FIntVector ToLocalPos(const FIntVector& GlobalPos, const FIntVector& ChunkPos)
	{
		return GlobalPos - ChunkPos * FGameConstants::ChunkSize;
	}
	
	static FIntVector ToSectionPos(const FVector& InPosition)
	{
		return FIntVector(
			FMath::FloorToInt(
				InPosition.X / (FGameConstants::ChunkSize * FGameConstants::ScaleMultiplier)),
			FMath::FloorToInt(
				InPosition.Y / (FGameConstants::ChunkSize * FGameConstants::ScaleMultiplier)),
			FMath::FloorToInt(
				InPosition.Z / (FGameConstants::ChunkSize * FGameConstants::ScaleMultiplier)));
	}

	static FIntVector2 ToChunkPos(const FVector& InPosition)
	{
		return FIntVector2(
			FMath::FloorToInt(
				InPosition.X / (FGameConstants::ChunkSize * FGameConstants::ScaleMultiplier)),
			FMath::FloorToInt(
				InPosition.Y / (FGameConstants::ChunkSize * FGameConstants::ScaleMultiplier)));
	}

	static int ChunkDistanceToPosition(const FIntVector2& ChunkPos, const FVector& Position)
	{
		const auto PlayerChunkPos = ToChunkPos(Position);
		const auto XDist = FMath::Abs(PlayerChunkPos.X - ChunkPos.X);
		const auto YDist = FMath::Abs(PlayerChunkPos.Y - ChunkPos.Y);
		return FMath::Max(XDist, YDist);
	}

	static int ChunkDistanceToPosition(const FIntVector& ChunkPos, const FVector& Position)
	{
		const auto PlayerChunkPos = ToSectionPos(Position);
		const auto XDist = FMath::Abs(PlayerChunkPos.X - ChunkPos.X);
		const auto YDist = FMath::Abs(PlayerChunkPos.Y - ChunkPos.Y);
		return FMath::Max(XDist, YDist);
	}

	static FIntVector ApplyResolution(const FIntVector& InPosition, const int8 Resolution)
	{
		return InPosition * (FGameConstants::ChunkSize / Resolution);
	}

	static FIntVector ExtractResolution(const FIntVector& InPosition, const int8 Resolution)
	{
		return InPosition / (FGameConstants::ChunkSize / Resolution);
	}

	static TSet<FIntVector2> GetPositionsAround(const FIntVector2 FromColumn,
	                                            const int Distance)
	{
		// TODO Add a distance only for height
		auto PositionsAroundPlayer = TSet<FIntVector2>();

		const auto LowestPos = FIntVector2{FromColumn.X - Distance, FromColumn.Y - Distance};
		const auto HighestPos = FIntVector2{FromColumn.X + Distance, FromColumn.Y + Distance};

		for (int X = LowestPos.X; X <= HighestPos.X; X++)
		{
			for (int Y = LowestPos.Y; Y <= HighestPos.Y; Y++)
			{
				const auto Pos = FIntVector2{X, Y};
				PositionsAroundPlayer.Add(Pos);
			}
		}

		return PositionsAroundPlayer;
	}

	static TArray<FIntVector2> GetNeighborPositions(const FIntVector2& OriginColumn)
	{
		return {
			OriginColumn + FIntVector2{1, 0},
			OriginColumn + FIntVector2{0, 1},
			OriginColumn + FIntVector2{-1, 0},
			OriginColumn + FIntVector2{0, -1}
		};
	}

	static uint8 GetLoDResolutionPerDistance(const int Distance)
	{
		if (Distance <= FGameConstants::DefaultLiveDistance)
		{
			return 16;
		}

		if (Distance <= FGameConstants::DefaultLoD8Distance)
		{
			return 8;
		}

		if (Distance <= FGameConstants::DefaultLoD4Distance)
		{
			return 4;
		}

		if (Distance <= FGameConstants::DefaultLoD2Distance)
		{
			return 2;
		}

		return 1;
	}
};
