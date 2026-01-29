#pragma once
#include "ObjectPool.h"

template <typename T, uint32 CHUNK_SIZE>
class TlsObjectPool
{
private:

	struct ChunkData;

	class Chunk
	{
	public:

		Chunk(const Chunk&) = delete;
		Chunk& operator=(const Chunk&) = delete;
		Chunk(Chunk&&) = delete;
		Chunk& operator=(Chunk&&) = delete;

		Chunk()
			:mAllocCount(0)
			, mFreeCount(0)
			, mChunkDataArray{}
		{
			for (int32 i = 0; i < CHUNK_SIZE; ++i)
			{
				mChunkDataArray[i].pChunk = this;
			}
		}
		
		~Chunk() = default;

		T* GetData()
		{
			if (mAllocCount >= CHUNK_SIZE)
			{
				return nullptr;
			}

			return &mChunkDataArray[mAllocCount++].data;
		}

		bool FreeAndResetIfAllFreed()
		{
			if (mFreeCount.fetch_add(1) == CHUNK_SIZE - 1)
			{
				chunkReset();

				return true;
			}

			return false;
		}

	private:

		void chunkReset()
		{
			mAllocCount = 0;
			mFreeCount.store(0);
		}

		int32 mAllocCount;
		std::atomic<int32> mFreeCount;
		ChunkData mChunkDataArray[CHUNK_SIZE];
	};

	struct ChunkData
	{
		T data;
		Chunk* pChunk;
	};

public:

	TlsObjectPool(const TlsObjectPool&) = delete;
	TlsObjectPool& operator=(const TlsObjectPool&) = delete;
	TlsObjectPool(TlsObjectPool&&) = delete;
	TlsObjectPool& operator=(TlsObjectPool&&) = delete;

	explicit TlsObjectPool()
		: mObjectPool(false, 0)
	{
	}

	~TlsObjectPool() = default;

	[[nodiscard]]
	T* Alloc()
	{
		if (mpTlsChunk == nullptr)
		{
			mpTlsChunk = mObjectPool.Alloc();
			return mpTlsChunk->GetData();
		}

		T* pData = mpTlsChunk->GetData();
		if (pData == nullptr)
		{
			mpTlsChunk = mObjectPool.Alloc();
			return mpTlsChunk->GetData();
		}

		return pData;
	}

	void Free(T* pData)
	{
		Chunk* pChunk = (reinterpret_cast<ChunkData*>(pData))->pChunk;

		if (pChunk->FreeAndResetIfAllFreed())
		{
			mObjectPool.Free(pChunk);
			mpTlsChunk = mObjectPool.Alloc();
		}
	}

	[[nodiscard]]
	int32 AllocCount() const
	{
		return mObjectPool.AllocCount();
	}

	[[nodiscard]]
	int32 PoolingCount() const
	{
		return mObjectPool.PoolingCount();
	}

private:

	static thread_local Chunk* mpTlsChunk;

	ObjectPool<Chunk> mObjectPool;
};

template <typename T, uint32 CHUNK_SIZE>
thread_local TlsObjectPool<T, CHUNK_SIZE>::Chunk* TlsObjectPool<T, CHUNK_SIZE>::mpTlsChunk = nullptr;