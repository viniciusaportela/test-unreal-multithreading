#pragma once

#include "CoreMinimal.h"
#include "Split.h"
#include "HierarchicalCol.generated.h"

USTRUCT(BlueprintType)
struct FHierarchicalCol
{
	GENERATED_BODY()

	FHierarchicalCol()
	{
	}

	FHierarchicalCol(const uint8 InSpan, const uint32 InBlockId) : Span(InSpan),
		BlockId(InBlockId)
	{
	}

	UPROPERTY()
	uint8 Span = 0;

	UPROPERTY()
	uint32 BlockId = -1;

	/**
	 * Check if the layer is uniform, meaning it is formed by a single block
	 */
	bool IsUniform() const
	{
		return Span != 0 && BlockId != -1;
	}

	TSplit<FHierarchicalCol> Split(const uint8 At,
	                               const uint8 ThisColY) const
	{
		// Have space between At and the Layer start, returning before
		TOptional<FHierarchicalCol> Before;
		if (At > ThisColY)
		{
			Before = FHierarchicalCol{static_cast<uint8>(At - ThisColY), BlockId};
		}

		const FHierarchicalCol Main = FHierarchicalCol{1, BlockId};

		TOptional<FHierarchicalCol> After;
		if (At + 1 < ThisColY + Span)
		{
			// -1 because we need to consider that we're removing the Main layer too
			After = FHierarchicalCol{static_cast<uint8>(ThisColY + Span - At - 1), BlockId};
		}

		return TSplit{Before, Main, After};
	}

	// DEV don't return array, instead modify the original array
	static TArray<FHierarchicalCol> MergeCols(TArray<FHierarchicalCol>& Cols)
	{
		TArray<FHierarchicalCol> MergedCols;

		uint32 LastBlockId = -1;
		uint8 LastSize = 0;

		for (int ColIdx = 0; ColIdx < Cols.Num(); ColIdx++)
		{
			auto& Col = Cols[ColIdx];

			if (Col.IsUniform())
			{
				if (LastBlockId == -1)
				{
					LastBlockId = Col.BlockId;
				}

				if (Col.BlockId == LastBlockId)
				{
					LastSize += Col.Span;
				}
				else
				{
					MergedCols.Add(FHierarchicalCol{LastSize, LastBlockId});
					LastBlockId = Col.BlockId;
					LastSize = Col.Span;
				}
			}
			else
			{
				if (LastSize != 0 && LastBlockId != -1)
				{
					MergedCols.Add(FHierarchicalCol{LastSize, LastBlockId});
					LastBlockId = -1;
					LastSize = 0;
				}

				MergedCols.Add(MoveTemp(Col));
			}

			// Is Last, add accumulated
			if (ColIdx == Cols.Num() - 1 && LastSize != 0 && LastBlockId != -1)
			{
				MergedCols.Add(FHierarchicalCol{LastSize, LastBlockId});
			}
		}

		return MergedCols;
	}
};
