#pragma once

#include "CoreMinimal.h"
#include "HierarchicalCol.h"
#include "FindResult.h"
#include "Split.h"
#include "MultiThreadTest/Constants/GameConstants.h"
#include "HierarchialRow.generated.h"

struct FGameConstants;

/**
 * Rows grows in the Y axis (right vector)
 * So, rows are indexed by X (first row is X=0)
 *
 * Rows are composed by columns which together span 16 blocks
 */
USTRUCT(BlueprintType)
struct FHierarchicalRow
{
	GENERATED_BODY()

	FHierarchicalRow()
	{
	}

	FHierarchicalRow(const uint8 InSpan, const uint32 InBlockId,
	                 const uint8 InResolution = FGameConstants::ChunkSize) : Span(InSpan),
		BlockId(InBlockId), Resolution(InResolution)
	{
	}

	explicit FHierarchicalRow(const TArray<FHierarchicalCol>& InCols,
	                          const uint8 InResolution = FGameConstants::ChunkSize) :
		Resolution(InResolution), Cols(InCols)
	{
	}

	UPROPERTY()
	uint8 Span = 1;

	UPROPERTY()
	uint32 BlockId = -1;

	UPROPERTY()
	uint8 Resolution = FGameConstants::ChunkSize;

	UPROPERTY()
	TArray<FHierarchicalCol> Cols;

	/**
	 * Check if the layer is uniform, meaning it is formed by a single block
	 */
	bool IsUniform() const
	{
		return Cols.Num() == 0 && BlockId != -1;
	}

	void Set(const uint32 InBlockId, const uint8 Y)
	{
		auto Result = FindCol(Y);
		checkf(Result.Found, TEXT("Col not found"));

		FHierarchicalCol* Col = Result.DataPtr ? Result.DataPtr : Result.Data.GetPtrOrNull();
		if (Col->Span > 1)
		{
			// Break, so we can manipulate only the col at position Y 
			const auto SplitCols = Col->Split(Y, Result.StartPos);

			if (Cols.Num())
			{
				Cols.RemoveAt(Result.StartIdx);
			}

			Cols.Insert(SplitCols.All(), Result.StartIdx);
			BlockId = -1;

			const auto Offset = SplitCols.Before.IsSet() ? 1 : 0;
			Col = &Cols[Result.StartIdx + Offset];
		}

		Col->BlockId = InBlockId;
	}

	void CascadeMinimalMerge()
	{
		checkf(!IsUniform(), TEXT("Row shouldn't be uniform"));

		auto Merged = FHierarchicalCol::MergeCols(Cols);

		if (Merged.Num() == 1)
		{
			BlockId = Merged[0].BlockId;
			Cols.Empty();
		}
		else
		{
			Cols = MoveTemp(Merged);
		}
	}

	// TODO extremely similar to other finds, unify?
	TFindResult<FHierarchicalCol> FindCol(const uint8 Y)
	{
		if (IsUniform())
		{
			return TFindResult(
				0, 0, FHierarchicalCol(Resolution, BlockId));
		}

		int CurColY = 0;
		for (int CurIdx = 0; CurIdx < Cols.Num(); CurIdx++)
		{
			FHierarchicalCol* Col = &Cols[CurIdx];
			if (CurColY + Col->Span - 1 >= Y)
			{
				return TFindResult(CurColY, CurIdx, Col);
			}

			CurColY += Col->Span;
		}

		return TFindResult<FHierarchicalCol>();
	}

	TSplit<FHierarchicalRow> Split(const uint8 At,
	                               const uint8 ThisRowX) const
	{
		// Have space between At and the Layer start, returning before
		TOptional<FHierarchicalRow> Before;
		if (At > ThisRowX)
		{
			Before = FHierarchicalRow{static_cast<uint8>(At - ThisRowX), BlockId, Resolution};
		}

		const FHierarchicalRow Main = FHierarchicalRow{1, BlockId, Resolution};

		TOptional<FHierarchicalRow> After;
		if (At + 1 < ThisRowX + Span)
		{
			// -1 because we need to consider that we're removing the Main layer too
			After = FHierarchicalRow{
				static_cast<uint8>(ThisRowX + Span - At - 1), BlockId, Resolution
			};
		}

		return TSplit{Before, Main, After};
	}

	FHierarchicalCol GetCol(const uint8 X) const
	{
		if (Cols.Num() == 0)
		{
			return FHierarchicalCol{1, BlockId};
		}

		int Index = 0;
		for (const auto& Col : Cols)
		{
			if (Index + Col.Span - 1 >= X)
			{
				return {1, Col.BlockId};
			}

			Index += Col.Span;
		}

		checkf(false, TEXT("Couldn't find X %d in row"), X);
		return FHierarchicalCol{Resolution, 0};
	}

	uint32 GetY(const uint8 Y) const
	{
		if (Cols.Num() == 0)
		{
			return BlockId;
		}

		int ColY = 0;
		for (const auto& Col : Cols)
		{
			if (ColY + Col.Span - 1 >= Y)
			{
				return Col.BlockId;
			}

			ColY += Col.Span;
		}

		return 0;
	}

	static TArray<FHierarchicalRow> MergeRows(TArray<FHierarchicalRow>& Rows)
	{
		if (Rows.Num() == 0)
		{
			return {};
		}

		const auto Resolution = Rows[0].Resolution;
		TArray<FHierarchicalRow> MergedRows;

		uint32 LastBlockId = -1;
		uint8 LastSize = 0;

		for (int RowIdx = 0; RowIdx < Rows.Num(); RowIdx++)
		{
			auto& Row = Rows[RowIdx];

			if (Row.IsUniform())
			{
				if (LastBlockId == -1)
				{
					LastBlockId = Row.BlockId;
				}

				if (Row.BlockId == LastBlockId)
				{
					LastSize += Row.Span;
				}
				else
				{
					MergedRows.Add(FHierarchicalRow{LastSize, LastBlockId, Resolution});
					LastBlockId = Row.BlockId;
					LastSize = Row.Span;
				}
			}
			else
			{
				if (LastSize != 0 && LastBlockId != -1)
				{
					MergedRows.Add(FHierarchicalRow{LastSize, LastBlockId, Resolution});
					LastBlockId = -1;
					LastSize = 0;
				}

				MergedRows.Add(MoveTemp(Row));
			}

			// Is Last, add accumulated
			if (RowIdx == Rows.Num() - 1)
			{
				MergedRows.Add(FHierarchicalRow{LastSize, LastBlockId, Resolution});
			}
		}

		return MergedRows;
	}
};
