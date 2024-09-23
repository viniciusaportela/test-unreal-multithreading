#pragma once

#include "CoreMinimal.h"
#include "FindResult.h"
#include "HierarchialLayer.h"
#include "HierarchialGrid.generated.h"

struct FLayersSplit;
struct FRangeNeighbors;
struct FGameConstants;

// TODO change orientation of rows to another axis if it's is more efficient?

USTRUCT(BlueprintType)
struct FHierarchicalGrid
{
	GENERATED_BODY()

	explicit FHierarchicalGrid(const uint8 InResolution = FGameConstants::ChunkSize) :
		Resolution(InResolution)
	{
	}

	explicit FHierarchicalGrid(const uint32 InBlockId,
	                           const uint8 InResolution = FGameConstants::ChunkSize) :
		BlockId(InBlockId), Resolution(InResolution)
	{
	}

	explicit FHierarchicalGrid(const TArray<FHierarchicalLayer>& InLayers,
	                           const uint8 InResolution = FGameConstants::ChunkSize) :
		Resolution(InResolution),
		Layers(InLayers)
	{
	}

	UPROPERTY()
	uint32 BlockId = 0;

	UPROPERTY(NotReplicated)
	uint8 Resolution = FGameConstants::ChunkSize;

	UPROPERTY()
	TArray<FHierarchicalLayer> Layers;

	bool IsUniform() const
	{
		return Layers.Num() == 0 && BlockId != -1;
	}

	void Set(const uint8 X, const uint8 Y, const uint8 Z, const uint32 InBlockId)
	{
		const auto CurrentBlockId = Get(X, Y, Z);
		if (CurrentBlockId == InBlockId)
		{
			return;
		}

		if (Resolution == 1)
		{
			BlockId = InBlockId;
			return;
		}

		auto Result = FindLayer(Z);
		checkf(Result.Found, TEXT("Layer not found"));

		FHierarchicalLayer* Layer = Result.DataPtr ? Result.DataPtr : Result.Data.GetPtrOrNull();
		if (Layer->Span > 1)
		{
			// Break, so we can manipulate only the layer at level Z
			const auto SplitLayers = Layer->Split(Z, Result.StartPos);

			// To prevent removing when there is none (when is uniform)
			if (Layers.Num())
			{
				Layers.RemoveAt(Result.StartIdx);
			}

			Layers.Insert(SplitLayers.All(), Result.StartIdx);
			
			BlockId = -1;
			
			const auto Offset = SplitLayers.Before.IsSet() ? 1 : 0;
			Layer = &Layers[Result.StartIdx + Offset];
		}

		Layer->Set(InBlockId, X, Y);
	}

	uint32 Get(const uint8 X, const uint8 Y, const uint8 Z) const
	{
		if (IsUniform())
		{
			return BlockId;
		}

		int Index = 0;
		for (const auto& Layer : Layers)
		{
			if (Index + Layer.Span > Z)
			{
				return Layer.GetXY(X, Y);
			}

			Index += Layer.Span;
		}

		return 0;
	}

	TFindResult<FHierarchicalLayer> FindLayer(const uint8 LayerZ) const
	{
		if (IsUniform())
		{
			return TFindResult{
				0, 0, FHierarchicalLayer{Resolution, BlockId, Resolution}
			};
		}

		uint8 CurLayerZ = 0;
		for (uint8 CurLayerIdx = 0; CurLayerIdx < Layers.Num(); CurLayerIdx++)
		{
			FHierarchicalLayer* Layer = const_cast<FHierarchicalLayer*>(&Layers[CurLayerIdx]);
			if (CurLayerZ + Layer->Span - 1 >= LayerZ)
			{
				return TFindResult{CurLayerZ, CurLayerIdx, Layer};
			}

			CurLayerZ += Layer->Span;
		}

		checkf(false, TEXT("LayerZ %d not found"), LayerZ);
		return TFindResult<FHierarchicalLayer>{};
	}
};
