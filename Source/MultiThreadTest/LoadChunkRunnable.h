#pragma once

#include "ChunkDataColumn.h"
#include "ThreadSafeQueue.h"

class UWorldGenerator;
class FLoadChunkRunnable;
class AChunk;
class UChunkRegistry;

struct FReturnData2
{
	FReturnData2(const TArray<FLoadChunkRunnable*>& InRunnables,
	             const TArray<FRunnableThread*>& InThreads):
		Runnables(InRunnables), Threads(InThreads)
	{
	}

	TArray<FLoadChunkRunnable*> Runnables;

	TArray<FRunnableThread*> Threads;
};

class FLoadChunkRunnable : public FRunnable
{
public:
	static FReturnData2 Create(
		UWorldGenerator* InWorldGenerator,
		const TSharedPtr<TThreadSafeQueue<FIntVector2>>& InLoadColumnQueue,
		const TSharedPtr<TThreadSafeQueue<FChunkDataColumn>>& InCreateColumnQueue,
		const int32 ThreadCount)
	{
		TArray<FLoadChunkRunnable*> Runnables;
		TArray<FRunnableThread*> Threads;

		for (int32 i = 0; i < ThreadCount; i++)
		{
			FLoadChunkRunnable* Runnable = new
				FLoadChunkRunnable(InWorldGenerator, InLoadColumnQueue,
				                   InCreateColumnQueue);
			Runnables.Add(Runnable);

			FRunnableThread* Thread = FRunnableThread::Create(
				Runnable, *FString::Printf(TEXT("LoadChunkRunnable %d"), i),
				0, TPri_AboveNormal);
			Threads.Add(Thread);
		}

		return FReturnData2(Runnables, Threads);
	}

	FLoadChunkRunnable(UWorldGenerator* InWorldGenerator,
	                   const TSharedPtr<TThreadSafeQueue<FIntVector2>>& InLoadColumnQueue,
	                   const TSharedPtr<TThreadSafeQueue<FChunkDataColumn>>&
	                   InCreateColumnQueue):
		WorldGenerator(InWorldGenerator),
		LoadColumnQueue(InLoadColumnQueue),
		CreateColumnQueue(InCreateColumnQueue)
	{
	}

	virtual uint32 Run() override;

	virtual void Stop() override;

private:
	UWorldGenerator* WorldGenerator;
	
	TSharedPtr<TThreadSafeQueue<FIntVector2>> LoadColumnQueue;

	TSharedPtr<TThreadSafeQueue<FChunkDataColumn>> CreateColumnQueue;

	FThreadSafeCounter StopTaskCounter;
};
