// ReSharper disable CppClangTidyClangDiagnosticPadded
#pragma once

#include "pch.h"
#include "ObjectPool.h"

template <uint32 ALLOC_SIZE, uint32 CHUNK_SIZE = 500>
class MemoryPool final
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
			for (uint32 i = 0; i < CHUNK_SIZE; ++i)
			{
				mChunkDataArray[i].pChunk = this;
			}
		}

		~Chunk() = default;

		void* GetData()
		{
			if (static_cast<uint32>(mAllocCount) >= CHUNK_SIZE)
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
		alignas(16) byte data[ALLOC_SIZE];
		Chunk* pChunk;
	};

public:

	MemoryPool(const MemoryPool&) = delete;
	MemoryPool& operator=(const MemoryPool&) = delete;
	MemoryPool(MemoryPool&&) = delete;
	MemoryPool& operator=(MemoryPool&&) = delete;

	explicit MemoryPool()
		:mObjectPool(false, 0)
	{
	}

	~MemoryPool() = default;

	[[nodiscard]]
	void* Alloc()
	{
		if (spTlsChunk == nullptr)
		{
			spTlsChunk = mObjectPool.Alloc();
		}

		void* pData = spTlsChunk->GetData();
		if (pData != nullptr && spTlsChunk->IsDataEmpty())
		{
			spTlsChunk = mObjectPool.Alloc();
		}

		return pData;
	}

	void Free(void* pData)
	{
		Chunk* pChunk = (static_cast<ChunkData*>(pData))->pChunk;

		if (pChunk->FreeWithIsAllFreed())
		{
			pChunk->ChunkReset();
			mObjectPool.Free(pChunk);
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

	inline static thread_local Chunk* spTlsChunk = nullptr;

	ObjectPool<Chunk> mObjectPool;
};
