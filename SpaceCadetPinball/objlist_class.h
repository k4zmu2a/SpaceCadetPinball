#pragma once

#include "memory.h"

template <class T>
class objlist_class
{
public:
	objlist_class(int sizeInt, int growSize)
	{
		ListPtr = memory::allocate<T*>(sizeInt);
		Count = 0;
		Size = sizeInt;
		GrowSize = growSize;
	}

	~objlist_class()
	{
		if (ListPtr)
			memory::free(ListPtr);
	}

	void Add(T* value)
	{
		if (Count == Size)
			Grow();
		if (Count >= Size)
			return;

		ListPtr[Count] = value;
		Count++;
	}

	void Grow()
	{
		if (!ListPtr)
			return;
		auto newSize = Count + GrowSize;
		if (newSize <= Size)
			return;

		auto newList = memory::realloc(ListPtr, sizeof(T*) * newSize);
		if (!newList)
			return;

		ListPtr = newList;
		Size = newSize;
	}

	int Delete(T* value)
	{
		for (auto index = Count - 1; index >= 0; index--)
		{
			if (ListPtr[index] == value)
			{
				ListPtr[index] = ListPtr[Count - 1];
				Count--;
				return 1;
			}
		}
		return 0;
	}

	T* Get(int index) const
	{
		if (index >= Count)
			return nullptr;
		return ListPtr[index];
	}

	int GetCount() const { return Count; }
	int GetSize() const { return Size; }
private:
	T** ListPtr;
	int GrowSize;
	int Size;
	int Count;
};
