// ReSharper disable CppMemberFunctionMayBeConst
#pragma once
#include "ISingleton.h"
#include "MemoryPool.h"
#include <tuple>
#include <utility> // std::integer_sequence
#include <array>

template <uint32 CHUNK_SIZE = 500>
class MemoryPoolManager final : public ISingleton<MemoryPoolManager<CHUNK_SIZE>>
{
private:

	static constexpr uint32 BUCKET_STRIDE = 32;
	static constexpr uint32 POOL_COUNT = 128;
	static constexpr uint32 MIN_ALLOC_SIZE = 32;
	static constexpr uint32 MAX_ALLOC_SIZE = 4096;

	static constexpr int64 CHECKSUM_CODE = 0xDEADBEEFBEFFDEAD;

public:

	friend class ISingleton<MemoryPoolManager<CHUNK_SIZE>>;

	MemoryPoolManager(const MemoryPoolManager&) = delete;
	MemoryPoolManager& operator=(const MemoryPoolManager&) = delete;
	MemoryPoolManager(MemoryPoolManager&&) = delete;
	MemoryPoolManager& operator=(MemoryPoolManager&&) = delete;

private:

	explicit MemoryPoolManager() = default;

public:

	virtual ~MemoryPoolManager() override = default;

	[[nodiscard]]
	void* Alloc(const uint64 size)
	{
		if (size > MAX_ALLOC_SIZE)
		{
			void* pData = mi_malloc(size + sizeof(uint64));

			*(static_cast<uint64*>(pData)) = CHECKSUM_CODE;

			pData = static_cast<byte*>(pData) + sizeof(uint64);

			return pData;
		}

		const uint64 index = getBucketIndex(size);

		const auto& table = getTable<AllocActor>(std::make_index_sequence<POOL_COUNT>{});

		return table[index](mBuckets);
	}

	void Free(void* pData, const uint64 size)
	{
		if (size > MAX_ALLOC_SIZE)
		{
			pData = static_cast<byte*>(pData) - sizeof(uint64);

			if (*(static_cast<uint64*>(pData)) == CHECKSUM_CODE)
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

private:

	static uint64 getBucketIndex(const uint64 size)
	{
		const uint64 index = (size <= MIN_ALLOC_SIZE) ? 0 : std::bit_width(size - 1) - 5;

		return index;
	}

	// ----------------------------------------------------------------------
	// [Type Helper] 구조체 템플릿을 이용해 Tuple 타입 생성 (에러 해결)
	// ----------------------------------------------------------------------
	template <typename SEQUENCE>
	struct TupleBuilder;

	template <uint64... INDEX>
	struct TupleBuilder<std::index_sequence<INDEX...>>
	{
		// (Is + 1) * 32 => 32, 64, ... 4096 타입 생성
		using type = std::tuple<MemoryPool<(INDEX + 1) * BUCKET_STRIDE>...>;
	};

	// 최종 Tuple 타입 정의
	using BucketsTuple = TupleBuilder<std::make_index_sequence<POOL_COUNT>>::type;

	class AllocActor
	{
	public:
		using FuncType = void* (*)(BucketsTuple&);

		// 실제 수행할 함수 (기존 AllocImpl)
		template <uint64 INDEX>
		static void* Do(BucketsTuple& buckets)
		{
			return std::get<INDEX>(buckets).Alloc();
		}
	};

	class FreeActor
	{
	public:
		using FuncType = void (*)(BucketsTuple&, void*);

		// 실제 수행할 함수 (기존 FreeImpl)
		template <uint64 INDEX>
		static void Do(BucketsTuple& buckets, void* pData) 
		{
			std::get<INDEX>(buckets).Free(pData);
		}
	};

	// --------------------------------------------------------------------------
	// [통합 테이블 생성기] 어떤 Action이든 처리하는 만능 함수
	// --------------------------------------------------------------------------
	template <typename ACTION, uint64... INDEX>
	static const auto& getTable(std::index_sequence<INDEX...>) 
	{
		static const std::array<typename ACTION::FuncType, POOL_COUNT> table
			= { &ACTION::template Do<INDEX>... }; // ★ 핵심: Action 안에 있는 Do<Is>를 꺼냄

		return table;
	}

	BucketsTuple mBuckets;
};
