// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldGenerator.h"

#include "ChunksStat.h"
#include "Constants/GameConstants.h"
#include "Structs/HierarchialGrid.h"

void UWorldGenerator::Generate(FIntVector2 ChunkPos, TArray<FHierarchicalGrid>& OutChunkData)
{
	OutChunkData.SetNum(FGameConstants::ChunksInZ);
	SCOPE_CYCLE_COUNTER(STAT_GenerateChunkGen);

	for (int X = 0; X < FGameConstants::ChunkSize; X++)
	{
		for (int Y = 0; Y < FGameConstants::ChunkSize; Y++)
		{
			SCOPE_CYCLE_COUNTER(STAT_GenerateChunkGenXY);

			constexpr float NoiseValue = 0.5;

			const int Height = FMath::RoundToInt(
				(NoiseValue + 1) * (FGameConstants::WorldHeight / 2));

			for (
				int ChunkZ = 0;
				ChunkZ < FMath::CeilToInt(Height / static_cast<float>(FGameConstants::ChunkSize));
				ChunkZ++
			)
			{
				const int WorldChunkZ = ChunkZ * FGameConstants::ChunkSize;
				const auto MaxHeightInThisChunk = FMath::Min(Height - WorldChunkZ,
				                                             FGameConstants::ChunkSize);

				for (int Z = 0; Z < MaxHeightInThisChunk; Z++)
				{
					SCOPE_CYCLE_COUNTER(STAT_GenerateChunkSet);
					OutChunkData[ChunkZ].Set(X, Y, Z, 1, false);
				}
			}
		}
	}

	// Merge all
	for (int Z = 0; Z < FGameConstants::ChunksInZ; Z++)
	{
		OutChunkData[Z].FullMerge();
	}
}
