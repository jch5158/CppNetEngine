#include "pch.h"
#include "MemoryAllocator.h"
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
	if (size > MAX_ALLOC_SIZE)
	{
		if (isValidChecksum(pData, size))
		{
			mi_free(pData);
		}
		else
		{
			CrashHandler::Crash();
		}

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
	uint64 index = 0;

	if (size <= MIN_ALLOC_SIZE)
	{
		index = 0;
	}
	else
	{
		index = static_cast<uint64>(std::bit_width(size - 1) - 5);
	}

	return index;
}

