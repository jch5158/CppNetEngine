#include "pch.h"
#include "MemoryAllocator.h"

#include <assert.h>

#include "CrashHandler.h"

void* MemoryAllocator::	Alloc(const uint64 size)
{
	if (size > MAX_ALLOC_SIZE)
	{
		void* pData = mi_malloc(size + sizeof(uint64));

		setChecksum(pData, size);

		return pData;
	}

	const uint64 index = getBucketIndex(size);

	const auto& table = getTable<AllocActor>(std::make_index_sequence<POOL_COUNT>{});

	return table[index](mBuckets);
}

void MemoryAllocator::Free(void* pData, const uint64 size)
{
	ASSERT(pData != nullptr, "MemoryAllocator::Free - pData is nullptr");

	if (size > MAX_ALLOC_SIZE)
	{
		if (!isValidChecksum(pData, size))
		{
			ASSERT(false, "MemoryAllocator::Free - Invalid checksum detected. Possible memory corruption.");

			return;
		}

		mi_free(pData);
		return;
	}

	const uint64 index = getBucketIndex(size);

	const auto& table = getTable<FreeActor>(std::make_index_sequence<POOL_COUNT>{});

	table[index](mBuckets, pData);
}

void MemoryAllocator::setChecksum(void* pData, const uint64 size)
{
	pData = static_cast<byte*>(pData) + (size - sizeof(uint64));

	*(static_cast<uint64*>(pData)) = CHECKSUM_CODE;
}

bool MemoryAllocator::isValidChecksum(void* pData, const uint64 size)
{
	pData = static_cast<byte*>(pData) + (size - sizeof(uint64));

	if (*(static_cast<uint64*>(pData)) == CHECKSUM_CODE)
	{
		return true;
	}

	return false;
}

uint64 MemoryAllocator::getBucketIndex(const uint64 size)
{
	const uint64 index = (size - 1) >> 5;

	return index;
}

