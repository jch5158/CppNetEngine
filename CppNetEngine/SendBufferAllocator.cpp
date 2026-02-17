#include "pch.h"
#include "SendBufferAllocator.h"

INetBufferRef SendBufferAllocator::Alloc(const int32 size)
{
	INetBufferRef pData = nullptr;

	if (size < 0 || size > MAX_SIZE)
	{
		return pData;
	}

	// 1. 작은 크기의 버퍼 요청일 경우
	if (size <= (THRESHOLD - SMALL_STRIDE))
	{
		const int32 index = getBucketIndex(size, SMALL_STRIDE);
		const auto& smallTable = getTable<SMALL_STRIDE>(std::make_index_sequence<SMALL_POOL_COUNT>{});
		pData = smallTable[index]();
	}
	else
	{
		// 2. 큰 크기의 버퍼 요청일 경우
		const int32 index = getBucketIndex(size, LARGE_STRIDE);
		const auto& largeTable = getTable<LARGE_STRIDE>(std::make_index_sequence<LARGE_POOL_COUNT>{});
		pData = largeTable[index]();
	}

	return pData;
}

int32 SendBufferAllocator::getBucketIndex(const int32 size, const int32 stride)
{
	const int32 index = (size - 1) / stride;

	return index;
}
