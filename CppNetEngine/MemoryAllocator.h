// ReSharper disable CppMemberFunctionMayBeConst
#pragma once
#include "ISingleton.h"
#include "MemoryPool.h"
#include <tuple>
#include <utility> // std::integer_sequence
#include <array>


class MemoryAllocator final : public ISingleton<MemoryAllocator>
{
private:

	static constexpr uint32 CHUNK_SIZE = 500;
	static constexpr uint32 BUCKET_STRIDE = 32;
	static constexpr uint32 POOL_COUNT = 128;
	static constexpr uint32 MIN_ALLOC_SIZE = 32;
	static constexpr uint32 MAX_ALLOC_SIZE = 4096;
	static constexpr uint64 CHECKSUM_CODE = 0xDEADBEEFBEFFDEAD;

public:

	friend class ISingleton<MemoryAllocator>;

	MemoryAllocator(const MemoryAllocator&) = delete;
	MemoryAllocator& operator=(const MemoryAllocator&) = delete;
	MemoryAllocator(MemoryAllocator&&) = delete;
	MemoryAllocator& operator=(MemoryAllocator&&) = delete;

private:

	explicit MemoryAllocator() = default;

public:

	virtual ~MemoryAllocator() override = default;

	[[nodiscard]]
	void* Alloc(const uint64 size);

	void Free(void* pData, const uint64 size);

private:

	static void setChecksum(void* pData, const uint64 size);

	static bool isValidChecksum(void* pData, const uint64 size);

	static uint64 getBucketIndex(const uint64 size);

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

	BucketsTuple mBuckets;  // NOLINT(clang-diagnostic-padded)
};
