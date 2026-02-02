// ReSharper disable CppMemberFunctionMayBeConst
#pragma once
#include "CrashHandler.h"
#include "ISingleton.h"
#include "MemoryPool.h"


template <uint32 CHUNK_SIZE = 500>
class MemoryPoolManager final : public ISingleton<MemoryPoolManager<CHUNK_SIZE>>
{
private:

	static constexpr uint32 MAX_ALLOC_SIZE = 8192;

public:

	friend class ISingleton<MemoryPoolManager<CHUNK_SIZE>>;

	MemoryPoolManager(const MemoryPoolManager&) = delete;
	MemoryPoolManager& operator=(const MemoryPoolManager&) = delete;
	MemoryPoolManager(MemoryPoolManager&&) = delete;
	MemoryPoolManager& operator=(MemoryPoolManager&&) = delete;

private:

	explicit MemoryPoolManager()
	{
	}

public:

	virtual ~MemoryPoolManager() override = default;

	[[nodiscard]]
	void* Alloc(const uint64 size)
	{
		if (size > MAX_ALLOC_SIZE)
		{
			CrashHandler::Crash();
		}

		void* pData = nullptr;

		// ReSharper disable once CppTooWideScope
		const uint64 idx = (size <= 32) ? 0 : std::bit_width(size - 1) - 5;

		switch (idx)
		{
		case 0:
			pData = mBuckets32.Alloc();
			break;
		case 1:
			pData = mBuckets64.Alloc();
			break;
		case 2:
			pData = mBuckets128.Alloc();
			break;
		case 3:
			pData = mBuckets256.Alloc();
			break;
		case 4:
			pData = mBuckets512.Alloc();
			break;
		case 5:
			pData = mBuckets1024.Alloc();
			break;
		case 6:
			pData = mBuckets2048.Alloc();
			break;
		case 7:
			pData = mBuckets4096.Alloc();
			break;
		case 8:
			pData = mBuckets8192.Alloc();
			break;
		default:
			CrashHandler::Crash();
			break;
		};

		return pData;
	}

	void Free(void* pData, const uint64 size)
	{
		if (size > MAX_ALLOC_SIZE)
		{
			CrashHandler::Crash();
		}

		// ReSharper disable once CppTooWideScope
		const uint64 idx = (size <= 32) ? 0 : std::bit_width(size - 1) - 5;

		switch (idx)
		{
		case 0:
			mBuckets32.Free(pData);
			break;
		case 1:
			mBuckets64.Free(pData);
			break;
		case 2:
			mBuckets128.Free(pData);
			break;
		case 3:
			mBuckets256.Free(pData);
			break;
		case 4:
			mBuckets512.Free(pData);
			break;
		case 5:
			mBuckets1024.Free(pData);
			break;
		case 6:
			mBuckets2048.Free(pData);
			break;
		case 7:
			mBuckets4096.Free(pData);
			break;
		case 8:
			mBuckets8192.Free(pData);
			break;
		default:
			CrashHandler::Crash();
			break;
		};
	}

private:
	MemoryPool<32> mBuckets32;
	MemoryPool<64> mBuckets64;
	MemoryPool<128> mBuckets128;
	MemoryPool<256> mBuckets256;
	MemoryPool<512> mBuckets512;
	MemoryPool<1024> mBuckets1024;
	MemoryPool<2048> mBuckets2048;
	MemoryPool<4096> mBuckets4096;
	MemoryPool<8192> mBuckets8192;
};
