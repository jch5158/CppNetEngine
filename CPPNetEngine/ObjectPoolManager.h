#pragma once
#include "ISingleton.h"
#include "TlsObjectPool.h"

template<typename T, uint32 CHUNK_SIZE>
class ObjectPoolManager final : public ISingleton<ObjectPoolManager<T, CHUNK_SIZE>>
{
public:

	friend class ISingleton<ObjectPoolManager>;

	ObjectPoolManager(const ObjectPoolManager&) = delete;
	ObjectPoolManager& operator=(const ObjectPoolManager&) = delete;
	ObjectPoolManager(ObjectPoolManager&&) = delete;
	ObjectPoolManager& operator=(ObjectPoolManager&&) = delete;

private:

	ObjectPoolManager() = default;

public:

	virtual ~ObjectPoolManager() override = default;

	[[nodiscard]]
	int32 AllocCount() const
	{
		return mTlsObjectPool.AllocCount();
	}

	[[nodiscard]]
	int32 PoolingCount() const
	{
		return mTlsObjectPool.PoolingCount();
	}

	[[nodiscard]]
	T* Alloc()
	{
		return mTlsObjectPool.Alloc();
	}

	void Free(T* pData)
	{
		mTlsObjectPool.Free(pData);
	}

private:
	
	TlsObjectPool<T, CHUNK_SIZE> mTlsObjectPool;
};

