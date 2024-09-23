#pragma once

#include "CoreMinimal.h"
#include "HierarchialRow.h"
#include "FindResult.h"
#include "Split.h"
#include "HierarchialLayer.generated.h"

struct FGameConstants;

USTRUCT(BlueprintType)
struct FHierarchicalLayer
{
	GENERATED_BODY()

	FHierarchicalLayer()
	{
	}

	FHierarchicalLayer(const uint8 InSpan, const uint32 InBlockId,
	                   const uint8 InResolution = FGameConstants::ChunkSize) : Span(InSpan),
		BlockId(InBlockId), Resolution(InResolution)
	{
	}

	explicit FHierarchicalLayer(const TArray<FHierarchicalRow>& InRows,
	                            const uint8 InResolution = FGameConstants::ChunkSize) :
		Resolution(InResolution), Rows(InRows)
	{
	}

	UPROPERTY()
	uint8 Span = 1;

	UPROPERTY()
	uint32 BlockId = -1;

	// TODO pass down instead of store
	UPROPERTY()
	uint8 Resolution = FGameConstants::ChunkSize;

	UPROPERTY()
	TArray<FHierarchicalRow> Rows;

	/**
	 * Check if the layer is uniform, meaning it is formed by a single block
	 */
	bool IsUniform() const
	{
		return Rows.Num() == 0 && BlockId != -1;
	}

	void Set(const uint32 InBlockId, const uint8 X, const uint8 Y)
	{
		auto Result = FindRow(X);
		checkf(Result.Found, TEXT("Row not found"));

		FHierarchicalRow* Row = Result.DataPtr ? Result.DataPtr : Result.Data.GetPtrOrNull();

		if (Row->Span > 1)
		{
			// Break, so we can manipulate only the row at level X
			const auto SplitRows = Row->Split(X, Result.StartPos);

			if (Rows.Num())
			{
				Rows.RemoveAt(Result.StartIdx);
			}

			Rows.Insert(SplitRows.All(), Result.StartIdx);

			BlockId = -1;

			const auto Offset = SplitRows.Before.IsSet() ? 1 : 0;
			Row = &Rows[Result.StartIdx + Offset];
		}

		Row->Set(InBlockId, Y);
	}

	void CascadeMinimalMergeStartingFrom(const uint8 X)
	{
		auto Result = FindRow(X);
		checkf(Result.Found, TEXT("Row not found"));

		FHierarchicalRow* Row = Result.DataPtr ? Result.DataPtr : Result.Data.GetPtrOrNull();

		Row->CascadeMinimalMerge();

		if (Row->IsUniform())
		{
			auto Merged = FHierarchicalRow::MergeRows(Rows);

			if (Merged.Num() == 1)
			{
				BlockId = Merged[0].BlockId;
				Rows.Empty();
			}
			else
			{
				Rows = MoveTemp(Merged);
			}
		}
	}

	TSplit<FHierarchicalLayer> Split(const uint8 At,
	                                 const uint8 ThisLayerZ) const
	{
		// Have space between At and the Layer start, returning before
		TOptional<FHierarchicalLayer> Before;
		if (At > ThisLayerZ)
		{
			Before = FHierarchicalLayer{static_cast<uint8>(At - ThisLayerZ), BlockId, Resolution};
		}

		const FHierarchicalLayer Main = FHierarchicalLayer{1, BlockId, Resolution};

		TOptional<FHierarchicalLayer> After;
		if (At + 1 < ThisLayerZ + Span)
		{
			// -1 because we need to consider that we're removing the Main layer too
			After = FHierarchicalLayer{
				static_cast<uint8>(ThisLayerZ + Span - At - 1), BlockId, Resolution
			};
		}

		return TSplit{Before, Main, After};
	}

	uint32 GetXY(const uint8 X, const uint8 Y) const
	{
		if (IsUniform())
		{
			return BlockId;
		}

		const auto FindRes = FindRow(X);
		if (FindRes.Found)
		{
			const auto Row = FindRes.DataPtr ? FindRes.DataPtr : FindRes.Data.GetPtrOrNull();
			return Row->GetY(Y);
		}

		return 0;
	}

	TFindResult<FHierarchicalRow> FindRow(const uint8 RowX) const
	{
		if (IsUniform())
		{
			return TFindResult(
				0, 0, FHierarchicalRow(Resolution, BlockId, Resolution));
		}

		int CurRowX = 0;
		for (int CurIdx = 0; CurIdx < Rows.Num(); CurIdx++)
		{
			FHierarchicalRow* Row = const_cast<FHierarchicalRow*>(&Rows[CurIdx]);

			if (CurRowX + Row->Span - 1 >= RowX)
			{
				return TFindResult(CurRowX, CurIdx, Row);
			}

			CurRowX += Row->Span;
		}

		return TFindResult<FHierarchicalRow>();
	}

	/**
	 * Return the row at the provided X,
	 * it will always return a row of span 1 (independently of the original row span)
	 */
	FHierarchicalRow GetUnitRow(const uint8 RowX) const
	{
		checkf(RowX < Resolution, TEXT("RowX out of bounds"));

		if (IsUniform())
		{
			return FHierarchicalRow(1, BlockId);
		}

		const auto FindRes = FindRow(RowX);
		const FHierarchicalRow* Row = FindRes.DataPtr
			                              ? FindRes.DataPtr
			                              : FindRes.Data.GetPtrOrNull();
		if (FindRes.Found)
		{
			if (Row->IsUniform())
			{
				return FHierarchicalRow(1, Row->BlockId);
			}

			return FHierarchicalRow(Row->Cols);
		}

		checkf(false, TEXT("RowIdx not found"));

		return FHierarchicalRow();
	}

	// TODO check performance
	// TODO use interface instead? (so merge row and merge col and merge layer has the implementation)
	static TArray<FHierarchicalLayer> MergeLayers(TArray<FHierarchicalLayer>& Layers)
	{
		if (Layers.Num() == 0)
		{
			return {};
		}

		const auto Resolution = Layers[0].Resolution;
		TArray<FHierarchicalLayer> Merged;

		uint32 LastBlockId = -1;
		uint8 LastSize = 0;

		for (int LayerIdx = 0; LayerIdx < Layers.Num(); LayerIdx++)
		{
			auto& Layer = Layers[LayerIdx];

			if (Layer.IsUniform())
			{
				if (LastBlockId == -1)
				{
					LastBlockId = Layer.BlockId;
				}

				if (Layer.BlockId == LastBlockId)
				{
					LastSize += Layer.Span;
				}
				else
				{
					Merged.Add(FHierarchicalLayer{LastSize, LastBlockId, Resolution});
					LastBlockId = Layer.BlockId;
					LastSize = Layer.Span;
				}
			}
			else
			{
				if (LastSize != 0 && LastBlockId != -1)
				{
					Merged.Add(FHierarchicalLayer{LastSize, LastBlockId, Resolution});
					LastBlockId = -1;
					LastSize = 0;
				}

				Merged.Add(MoveTemp(Layer));
			}

			// Is Last, add accumulated
			if (LayerIdx == Layers.Num() - 1 && LastSize != 0 && LastBlockId != -1)
			{
				Merged.Add(FHierarchicalLayer{LastSize, LastBlockId, Resolution});
			}
		}

		return Merged;
	}
};
