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

    // 2. 리바인딩 구조체 (이것도 public이어야 함)
    template <typename U>
    struct rebind
    {
        using other = SharedPtrAllocator<U>;
    };

    // 3. 생성자
    explicit SharedPtrAllocator() = default;
    
	template <typename U>
	explicit SharedPtrAllocator(const SharedPtrAllocator<U>&){}

    // 4. 할당/해제 함수
    T* allocate(const uint64 size)
	{
		const uint64 objSize = sizeof(T) * size;

        T* p = static_cast<T*>(MemoryPoolManager<>::GetInstance().Alloc(objSize));

        return p;
	}

	void deallocate(T* p, const uint64 size)
	{
        const uint64 objSize = sizeof(T) * size;

        MemoryPoolManager<>::GetInstance().Free(p, objSize);
	}

public:

};
