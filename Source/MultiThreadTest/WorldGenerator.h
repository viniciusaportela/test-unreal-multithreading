// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Structs/HierarchialGrid.h"
#include "WorldGenerator.generated.h"

class UFastNoiseWrapper;

/**
 * 
 */
UCLASS(BlueprintType)
class MULTITHREADTEST_API UWorldGenerator : public UObject
{
	GENERATED_BODY()

public:
	virtual void Generate(FIntVector2 ChunkPos, TArray<FHierarchicalGrid>& OutChunkData);
};
