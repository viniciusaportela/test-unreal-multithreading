#include "LoadChunkRunnable.h"

#include "ChunkDataColumn.h"
#include "ChunksStat.h"
#include "WorldGenerator.h"
#include "Constants/GameConstants.h"

struct FRenderData;

uint32 FLoadChunkRunnable::Run()
{
	while (StopTaskCounter.GetValue() == 0)
	{
		LoadColumnQueue->CriticalSection.Lock();
		if (LoadColumnQueue.Get()->IsEmptyWithoutLock())
		{
			LoadColumnQueue->CriticalSection.Unlock();
			FPlatformProcess::Sleep(0.01f);
			continue;
		}

		{
			SCOPE_CYCLE_COUNTER(STAT_GenerateChunk);

			auto ColumnPosOpt = LoadColumnQueue.Get()->DequeueSafeWithoutLock();
			LoadColumnQueue->CriticalSection.Unlock();

			if (ColumnPosOpt.IsSet())
			{
				const auto ColumnPos = ColumnPosOpt.GetValue();
				FChunkDataColumn ColumnData{ColumnPos};
				for (int Z = 0; Z < FGameConstants::ChunksInZ; Z++)
				{
					TArray<FHierarchicalGrid> Grids;
					WorldGenerator->Generate(ColumnPos, Grids);
					ColumnData.ChunkDatas = MoveTemp(Grids);
				}

				CreateColumnQueue->CriticalSection.Lock();
				CreateColumnQueue.Get()->EnqueueWithoutLock(MoveTemp(ColumnData));
				CreateColumnQueue->CriticalSection.Unlock();
			}
		}

		FPlatformProcess::Sleep(0.01f);
	}

	return 0;
}

void FLoadChunkRunnable::Stop()
{
	StopTaskCounter.Increment();
}
