#pragma once
#include "MemoryAllocator.h"

template <typename T>
class StlAllocator
{
public:
	using value_type = T;

	explicit StlAllocator() = default;
	~StlAllocator() = default;

	template <typename U>
	explicit StlAllocator(const StlAllocator<U>&) {}

	// ReSharper disable once CppInconsistentNaming
	static T* allocate(const uint64 size)
	{
		void * pData = MemoryAllocator::GetInstance().Alloc(size * sizeof(T));

		return static_cast<T*>(pData);
	}

	// ReSharper disable once CppInconsistentNaming
	static void deallocate(T* pData, const uint64 size)
	{
		MemoryAllocator::GetInstance().Free(pData, size * sizeof(T));
	}
};

