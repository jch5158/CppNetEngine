#pragma once

#include "pch.h"
#include "ObjectPool.h"

template <typename T, uint32 CHUNK_SIZE = 500>
class TlsObjectPool final
{
private:

	struct ChunkData;

	class Chunk final
	{
	public:

		Chunk(const Chunk&) = delete;
		Chunk& operator=(const Chunk&) = delete;
		Chunk(Chunk&&) = delete;
		Chunk& operator=(Chunk&&) = delete;

		explicit Chunk()
			:mAllocCount(0)
			, mFreeCount(0)
			, mChunkDataArray{}
		{
			for (int32 i = 0; i < CHUNK_SIZE; ++i)
			{
				mChunkDataArray[i].pChunk = this;
			}
		}
		
		~Chunk()
		{
			ChunkReset();
		}

		T* GetData()
		{
			if (mAllocCount >= CHUNK_SIZE)
			{
				return nullptr;
			}

			return &mChunkDataArray[mAllocCount++].data;
		}

		bool IsDataEmpty() const
		{
			return mAllocCount == CHUNK_SIZE;
		}

		bool FreeWithIsAllFreed()
		{
			if (mFreeCount.fetch_add(1) == CHUNK_SIZE - 1)
			{
				return true;
			}

			return false;
		}

		void ChunkReset()
		{
			mAllocCount = 0;
			mFreeCount.store(0);
		}

	public:

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
		: mAllocCount(0)
		, mObjectPool(false, 0)
	{
		static_assert(std::is_class_v<T>, "T is not class type.");
	}

	~TlsObjectPool() = default;

	[[nodiscard]]
	T* Alloc()
	{
		mAllocCount.fetch_add(1);

		if (spTlsChunk == nullptr)
		{
			spTlsChunk = mObjectPool.Alloc();
		}

		T* pData = spTlsChunk->GetData();
		if (pData != nullptr && spTlsChunk->IsDataEmpty())
		{
			spTlsChunk = mObjectPool.Alloc();
		}

		return pData;
	}

	void Free(T* pData)
	{
		Chunk* pChunk = (reinterpret_cast<ChunkData*>(pData))->pChunk;

		if (pChunk->FreeWithIsAllFreed())
		{
			pChunk->ChunkReset();
			mObjectPool.Free(pChunk);
		}

		mAllocCount.fetch_sub(1);
	}

	void AllFree()
	{
		spTlsChunk->ChunkReset();
		mObjectPool.Free(spTlsChunk);
		spTlsChunk = nullptr;
	}

	[[nodiscard]]
	int32 AllocCount() const
	{
		return mAllocCount.load();
	}

private:

	inline static thread_local Chunk* spTlsChunk = nullptr;

	std::atomic<int32> mAllocCount;

	ObjectPool<Chunk> mObjectPool;
};