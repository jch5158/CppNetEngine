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

	static constexpr uint32 SMALL_STRIDE = 256;
	static constexpr uint32 LARGE_STRIDE = 4096;
	static constexpr uint32 THRESHOLD = 4096;
	static constexpr uint32 MAX_SIZE = 65536;
	static constexpr uint32 SMALL_POOL_COUNT = (THRESHOLD / SMALL_STRIDE) - 1;
	static constexpr uint32 LARGE_POOL_COUNT = (MAX_SIZE / LARGE_STRIDE);
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

	[[nodiscard]]
	static bool isValidChecksum(void* pData, const uint64 size);

	[[nodiscard]]
	static uint64 getBucketIndex(const uint64 size, const uint32 stride);


	template <uint32 STRIDE, typename SEQUENCE>
	struct TupleBuilder
	{
	};

	template <uint32 STRIDE, uint64... INDEX>
	struct TupleBuilder<STRIDE, std::index_sequence<INDEX...>>
	{
		using type = std::tuple<MemoryPool<(INDEX + 1)* STRIDE>...>;
	};

	template <typename T>
	class AllocActor
	{
	public:
		using FuncType = void* (*)(T&);

		template <uint64 INDEX>
		static void* Do(T& buckets)
		{
			return std::get<INDEX>(buckets).Alloc();
		}
	};

	template <typename T>
	class FreeActor
	{
	public:
		using FuncType = void (*)(T&, void*);

		template <uint64 INDEX>
		static void Do(T& buckets, void* pData) 
		{
			std::get<INDEX>(buckets).Free(pData);
		}
	};

	template <typename ACTION, uint64... INDEX>
	static const auto& getTable(std::index_sequence<INDEX...>) 
	{
		constexpr int32 count = static_cast<int32>(sizeof...(INDEX));

		static const std::array<typename ACTION::FuncType, count> table
			= { &ACTION::template Do<INDEX>... };

		return table;
	}

	using SmallBucketsTuple = TupleBuilder<SMALL_STRIDE, std::make_index_sequence<SMALL_POOL_COUNT>>::type;
	using LargeBucketsTuple = TupleBuilder<LARGE_STRIDE, std::make_index_sequence<LARGE_POOL_COUNT>>::type;

	using SmallAllocActor = AllocActor<SmallBucketsTuple>;
	using SmallFreeActor = FreeActor<SmallBucketsTuple>;
	using LargeAllocActor = AllocActor<LargeBucketsTuple>;
	using LargeFreeActor = FreeActor<LargeBucketsTuple>;

	SmallBucketsTuple mSmallBuckets;
	LargeBucketsTuple mLargeBuckets;
};
