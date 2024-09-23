#pragma once

#include "CoreMinimal.h"
#include "Constants/GameConstants.h"
#include "ChunkDataColumn.generated.h"

struct FHierarchicalGrid;

USTRUCT(BlueprintType)
struct FChunkDataColumn
{
	GENERATED_BODY()

	FChunkDataColumn(): ColumnPos()
	{
		ChunkDatas.SetNum(FGameConstants::ChunkSize);
	}

	explicit FChunkDataColumn(const FIntVector2 InColumnPos): ColumnPos(InColumnPos)
	{
		ChunkDatas.SetNum(FGameConstants::ChunkSize);
	}

	UPROPERTY()
	FIntVector2 ColumnPos;

	UPROPERTY()
	TArray<FHierarchicalGrid> ChunkDatas;
};
