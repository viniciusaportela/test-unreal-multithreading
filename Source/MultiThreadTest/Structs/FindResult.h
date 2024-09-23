#pragma once

template <typename T>
struct TFindResult
{
	TFindResult()
	{
	}

	TFindResult(const uint8 InStartPos, const uint8 InStartIdx, T* InDataPtr) :
		Found(true),
		StartPos(InStartPos),
		StartIdx(InStartIdx),
		DataPtr(InDataPtr)
	{
	}

	TFindResult(const uint8 InStartPos, const uint8 InStartIdx, T InData) :
		Found(true),
		StartPos(InStartPos),
		StartIdx(InStartIdx),
		Data(InData)
	{
	}

	bool Found = false;

	uint8 StartPos = -1;

	uint8 StartIdx = -1;

	/**
	 * Generally used if there is not a specific layer with the given params but instead the
	 * whole structure is filled with the same block (therefore, the array is empty and we
	 * don't have a pointer to return)
	 */
	TOptional<T> Data;

	T* DataPtr = nullptr;
};
