#pragma once
#include "ObjectPool.h"

template <typename T>
class TlsObjectPool
{
private:

	class Chunk
	{
	public:

		Chunk(const Chunk&) = delete;
		Chunk& operator=(const Chunk&) = delete;
		Chunk(Chunk&&) = delete;
		Chunk& operator=(Chunk&&) = delete;

		Chunk() = default;
		~Chunk() = default;
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
		:mObjectPool(false, 0)
	{
	}

	~TlsObjectPool() = default;

private:
	
	ObjectPool<Chunk> mObjectPool;
};
