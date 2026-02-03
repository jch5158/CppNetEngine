// ReSharper disable CppInconsistentNaming
#pragma once
#include "CrashHandler.h"
#include "MemoryPool.h"
#include "MemoryPoolManager.h"

template <typename T>
class SharedPtrAllocator final
{
public:

    using value_type = T;

	explicit SharedPtrAllocator() = default;
    
	template <typename U>
	explicit SharedPtrAllocator(const SharedPtrAllocator<U>&) {}

    // 4. 할당/해제 함수
    static T* allocate(const uint64 size)
	{
		const uint64 sharedPtrSize = sizeof(T) * size;

        T* p = static_cast<T*>(MemoryPoolManager<>::GetInstance().Alloc(sharedPtrSize));

        return p;
	}

	static void deallocate(T* p, const uint64 size)
	{
        const uint64 objSize = sizeof(T) * size;

        MemoryPoolManager<>::GetInstance().Free(p, objSize);
	}
};
