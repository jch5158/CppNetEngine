// ReSharper disable CppInconsistentNaming
#pragma once
#include "MemoryAllocator.h"

template <typename T>
class SharedPtrAllocator final
{
public:

    using value_type = T;

	explicit SharedPtrAllocator() = default;
	explicit SharedPtrAllocator(const SharedPtrAllocator&) = default;
	SharedPtrAllocator& operator=(const SharedPtrAllocator&) = default;
	~SharedPtrAllocator() = default;
    
	template <typename U>
	explicit SharedPtrAllocator(const SharedPtrAllocator<U>&) {}

    // 4. 할당/해제 함수
    static T* allocate(const uint64 size)
	{
		const uint64 sharedPtrSize = sizeof(T) * size;

        T* ptr = static_cast<T*>(MemoryAllocator::GetInstance().Alloc(sharedPtrSize));

        return ptr;
	}

	static void deallocate(T* ptr, const uint64 size)
	{
        const uint64 objSize = sizeof(T) * size;

        MemoryAllocator::GetInstance().Free(ptr, objSize);
	}
};
