#pragma once
#include "ObjectAllocator.h"
#include "MemoryAllocator.h"
#include "UniquePtrUtils.h"
#include "SharedPtrUtils.h"
#include "SendBufferAllocator.h"

namespace cpp_net_engine
{
	template <typename T, typename... Args>
	T* ObjectAlloc(Args&&... args)
	{
		return ObjectAllocator<T>::GetInstance().Alloc(std::forward<Args>(args)...);
	}

	inline void* MemoryAlloc(const int64 size)
	{
		return MemoryAllocator::GetInstance().Alloc(size);
	}

	inline INetBufferRef MakeSendBuffer(const int32 size)
	{
		return SendBufferAllocator::Alloc(size);
	}

	template <typename T, typename... Args>
	UniquePtr<T> MakeUniquePtr(Args&&... args)
	{
		return UniquePtrUtils<T>::Alloc(std::forward<Args>(args)...);
	}

	template <typename T>
	UniquePtr<T[]> MakeUniquePtrArray()
	{
		return UniquePtrUtils<T[]>::Alloc();
	}

	template <typename T, typename... Args>
	std::shared_ptr<T> MakeSharedPtr(Args&&... args)
	{
		return SharedPtrUtils::Alloc<T>(std::forward<Args>(args)...);
	}
}