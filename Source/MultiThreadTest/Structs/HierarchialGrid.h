#pragma once

#include <functional>

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

	void Set(const uint8 X, const uint8 Y, const uint8 Z, const uint32 InBlockId, const bool bTriggerMerge = true)
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

		if (bTriggerMerge)
		{
			CascadeMinimalMergeStartingFrom(X, Z);	
		}
	}

	/**
	 * Tries to merge all the hierarchy from the columns up to the grid level
	 */
	void FullMerge()
	{
		// Go for all inner pieces and merge way up
		for (int LayerIdx = 0; LayerIdx < Layers.Num(); LayerIdx++)
		{
			auto& Layer = Layers[LayerIdx];

			for (int RowIdx = 0; RowIdx < Layer.Rows.Num(); RowIdx++)
			{
				auto& Row = Layer.Rows[RowIdx];

				auto MergedCols = FHierarchicalCol::MergeCols(Row.Cols);

				if (MergedCols.Num() == 1)
				{
					Row.BlockId = MergedCols[0].BlockId;
					Row.Cols.Empty();
				} else
				{
					Row.Cols = MoveTemp(MergedCols);
				}
			}

			auto MergedRows = FHierarchicalRow::MergeRows(Layer.Rows);

			if (MergedRows.Num() == 1)
			{
				Layer.BlockId = MergedRows[0].BlockId;
				Layer.Rows.Empty();
			} else
			{
				Layer.Rows = MoveTemp(MergedRows);
			}
		}

		auto Merged = FHierarchicalLayer::MergeLayers(Layers);

		if (Merged.Num() == 1)
		{
			BlockId = Merged[0].BlockId;
			Layers.Empty();
		}
		else
		{
			Layers = MoveTemp(Merged);
		}
	}
	
	/**
	 * Try merging from a starting Col going up until the Grid level. If it couldn't merge at any point,
	 * it will skip any subsequent merge try and return.
	 *
	 * WARNING: It's expected that the X,Y,Z and a separated block (we don't break up unified hierarchical containers)
	 */
	void CascadeMinimalMergeStartingFrom(const uint8 X, const uint8 Z)
	{
		auto Result = FindLayer(Z);
		checkf(Result.Found, TEXT("Layer not found"));

		FHierarchicalLayer* Layer = Result.Data.IsSet()
			                            ? Result.Data.GetPtrOrNull()
			                            : Result.DataPtr;

		Layer->CascadeMinimalMergeStartingFrom(X);

		if (Layer->IsUniform())
		{
			// Try merge here
			auto Merged = FHierarchicalLayer::MergeLayers(Layers);

			if (Merged.Num() == 1)
			{
				BlockId = Merged[0].BlockId;
				Layers.Empty();
			}
			else
			{
				Layers = MoveTemp(Merged);
			}
		}
	}

	void Fill(const uint32 InBlockId)
	{
		BlockId = InBlockId;
		Layers.Empty();
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

	FHierarchicalLayer GetLayer(const uint8 LayerZ) const
	{
		TFindResult<FHierarchicalLayer> Result = FindLayer(LayerZ);

		if (Result.Found)
		{
			return Result.Data.IsSet() ? Result.Data.GetValue() : *Result.DataPtr;
		}

		checkf(false, TEXT("LayerIdx not found"));
		return FHierarchicalLayer{0, 0};
	}

	void ForEachPiece(
		const std::function<void(FIntVector Size, FIntVector Offset, uint32 BlockId)>& Callback)
	const
	{
		// Is grid filled with same block
		if (IsUniform())
		{
			Callback(FIntVector{
				         Resolution, Resolution,
				         Resolution
			         },
			         FIntVector{0, 0, 0}, BlockId);
			return;
		}

		auto LayerZ = 0;
		for (int LayerIdx = 0; LayerIdx < Layers.Num(); LayerIdx++)
		{
			const auto Layer = Layers[LayerIdx];

			if (Layer.IsUniform())
			{
				Callback(FIntVector{
					         Resolution, Resolution,
					         Layer.Span
				         },
				         FIntVector{0, 0, LayerZ}, Layer.BlockId);

				LayerZ += Layer.Span;
				continue;
			}

			auto RowX = 0;
			for (int RowIdx = 0; RowIdx < Layer.Rows.Num(); RowIdx++)
			{
				const auto Row = Layer.Rows[RowIdx];

				if (Row.IsUniform())
				{
					Callback(FIntVector{Row.Span, Resolution, 1},
					         FIntVector{RowX, 0, LayerZ}, Row.BlockId);

					RowX += Row.Span;
					continue;
				}

				auto ColY = 0;
				for (int ColIdx = 0; ColIdx < Row.Cols.Num(); ColIdx++)
				{
					const auto Col = Row.Cols[ColIdx];

					Callback(FIntVector{1, Col.Span, 1}, FIntVector{RowX, ColY, LayerZ},
					         Col.BlockId);

					ColY += Col.Span;
				}

				RowX += Row.Span;
			}

			LayerZ += Layer.Span;
		}
	}

	FHierarchicalGrid GenerateLoD(const uint8 InResolution) const
	{
		checkf(Resolution == FGameConstants::ChunkSize,
		       TEXT("This function should be called on a full grid (Resolution == ChunkSize)"));
		auto OutGrid = FHierarchicalGrid{0, InResolution};
		const auto Jump = FGameConstants::ChunkSize / InResolution;

		for (int X = 0; X < FGameConstants::ChunkSize; X += Jump)
		{
			for (int Y = 0; Y < FGameConstants::ChunkSize; Y += Jump)
			{
				for (int Z = 0; Z < FGameConstants::ChunkSize; Z += Jump)
				{
					const auto BlockIdInPosition = Get(X, Y, Z);
					OutGrid.Set(X / Jump, Y / Jump, Z / Jump, BlockIdInPosition);
				}
			}
		}

		return OutGrid;
	}
};
