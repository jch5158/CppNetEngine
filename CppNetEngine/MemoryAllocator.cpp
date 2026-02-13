#include "pch.h"
#include "MemoryAllocator.h"

void* MemoryAllocator::	Alloc(const uint64 size)
{
	if (size == 0)
	{
		return nullptr;
	}

	void* pData = nullptr;

	if (size <= (THRESHOLD - SMALL_STRIDE))
	{
		const uint64 index = getBucketIndex(size, SMALL_STRIDE);

		const auto& table = getTable<SmallAllocActor>(std::make_index_sequence<SMALL_POOL_COUNT>{});

		pData = table[index](mSmallBuckets);
	}
	else if (size <= MAX_SIZE)
	{
		const uint64 index = getBucketIndex(size, LARGE_STRIDE);

		const auto& table = getTable<LargeAllocActor>(std::make_index_sequence<LARGE_POOL_COUNT>{});

		pData = table[index](mLargeBuckets);
	}
	else
	{
		pData = mi_malloc(size + sizeof(uint64));

		setChecksum(pData, size);
	}

	return pData;
}

void MemoryAllocator::Free(void* pData, const uint64 size)
{
	ASSERT(pData != nullptr, "MemoryAllocator::Free - pData is nullptr");

	if (size <= (THRESHOLD - SMALL_STRIDE))
	{
		const uint64 index = getBucketIndex(size, SMALL_STRIDE);

		const auto& table = getTable<SmallFreeActor>(std::make_index_sequence<SMALL_POOL_COUNT>{});

		table[index](mSmallBuckets, pData);
	}
	else if (size <= MAX_SIZE)
	{
		const uint64 index = getBucketIndex(size, LARGE_STRIDE);

		const auto& table = getTable<LargeFreeActor>(std::make_index_sequence<LARGE_POOL_COUNT>{});

		table[index](mLargeBuckets, pData);
	}
	else
	{
		if (!isValidChecksum(pData, size))
		{
			ASSERT(false, "MemoryAllocator::Free - Invalid checksum detected. Possible memory corruption.");

			return;
		}

		mi_free(pData);
	}
}

void MemoryAllocator::setChecksum(void* pData, const uint64 size)
{
	auto* const checksumOffset = reinterpret_cast<uint64*>(static_cast<byte*>(pData) + size);

	*checksumOffset = CHECKSUM_CODE;
}

bool MemoryAllocator::isValidChecksum(void* pData, const uint64 size)
{
	const auto* const checksumOffset = reinterpret_cast<uint64*>(static_cast<byte*>(pData) + size);

	return *checksumOffset == CHECKSUM_CODE;
}

uint64 MemoryAllocator::getBucketIndex(const uint64 size, const uint32 stride)
{
	const int64 index = (static_cast<int64>(size) - 1) / stride;

	return index;
}
