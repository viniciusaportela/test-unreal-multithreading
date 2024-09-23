#pragma once

// TODO make it more efficient (remove from front, but no resize neither move all elements, move head)
template <typename T>
class TThreadSafeQueue
{
public:
	void Enqueue(const T Item)
	{
		FScopeLock Lock(&CriticalSection);
		Queue.Add(Item);
	}

	void EnqueueWithoutLock(const T&& Item)
	{
		Queue.Add(Item);
	}

	T Dequeue()
	{
		FScopeLock Lock(&CriticalSection);
		return Queue.Pop();
	}

	void DequeueMany(TArray<T>& OutRes, int Count)
	{
		FScopeLock Lock(&CriticalSection);
		OutRes.Reserve(Count);
		for (int i = 0; i < Count; i++)
		{
			if (Queue.Num() == 0)
			{
				break;
			}
			OutRes.Add(Queue.Pop());
		}
	}

	TOptional<T> DequeueSafeWithoutLock()
	{
		return Queue.Pop();
	}

	TOptional<T> DequeueSafe()
	{
		FScopeLock Lock(&CriticalSection);
		if (Queue.Num() == 0)
		{
			return T();
		}

		return Queue.Pop();
	}

	int Num()
	{
		return Queue.Num();
	}

	bool IsEmpty() const
	{
		TQueue<FIntVector2> Q;
		FScopeLock Lock(&CriticalSection);
		return Queue.IsEmpty();
	}

	bool IsEmptyWithoutLock() const
	{
		return Queue.IsEmpty();
	}

	TArray<T> ArrayCopy()
	{
		FScopeLock Lock(&CriticalSection);
		return Queue;
	}

	mutable FCriticalSection CriticalSection;

private:
	TArray<T> Queue;
};
