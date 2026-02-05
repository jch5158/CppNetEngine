#pragma once

#include "ISingleton.h"
#include "TlsObjectPool.h"

template <typename T, uint32 CHUNK_SIZE = 500>
class ObjectAllocator final : public ISingleton<ObjectAllocator<T, CHUNK_SIZE>>
{
public:

	friend class ISingleton<ObjectAllocator>;

	ObjectAllocator(const ObjectAllocator&) = delete;
	ObjectAllocator& operator=(const ObjectAllocator&) = delete;
	ObjectAllocator(ObjectAllocator&&) = delete;
	ObjectAllocator& operator=(ObjectAllocator&&) = delete;

private:

	ObjectAllocator()
		:mTlsObjectPool()
	{
		static_assert(std::is_class_v<T>, "T is not class type.");
	}

public:

	virtual ~ObjectAllocator() override = default;

	[[nodiscard]]
	int32 AllocCount() const
	{
		return mTlsObjectPool.AllocCount();
	}

	[[nodiscard]]
	T* Alloc()
	{
		T* pData = mTlsObjectPool.Alloc();

		return pData;
	}

	void Free(T* pData)
	{
		mTlsObjectPool.Free(pData);
	}

private:
	
	TlsObjectPool<T, CHUNK_SIZE> mTlsObjectPool;
};

