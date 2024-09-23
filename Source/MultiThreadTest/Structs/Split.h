#pragma once

#include "CoreMinimal.h"

template <typename T>
struct TSplit
{
	TSplit()
	{
	}

	TSplit(const TOptional<T>& InBefore, const T& InMain, const TOptional<T>& InAfter) :
		Before(InBefore),
		Main(InMain),
		After(InAfter)
	{
	}

	TOptional<T> Before;

	T Main;

	TOptional<T> After;

	TArray<T> All() const
	{
		TArray<T> AllItems;

		if (Before.IsSet())
		{
			AllItems.Add(Before.GetValue());
		}

		AllItems.Add(Main);

		if (After.IsSet())
		{
			AllItems.Add(After.GetValue());
		}

		return AllItems;
	}
};
