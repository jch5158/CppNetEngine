#pragma once

#include "INetBuffer.h"
#include "NetSendBuffer.h"
#include "SharedPtrUtils.h"
#include <tuple>
#include <utility> // std::integer_sequence
#include <array>

class SendBufferAllocator
{
private:
	static constexpr int32 SMALL_STRIDE = 256;
	static constexpr int32 LARGE_STRIDE = 4096;
	static constexpr int32 THRESHOLD = 4096;
	static constexpr int32 MAX_SIZE = 65536;
	static constexpr int32 SMALL_POOL_COUNT = (THRESHOLD / SMALL_STRIDE) - 1;
	static constexpr int32 LARGE_POOL_COUNT = (MAX_SIZE / LARGE_STRIDE);

public:

	SendBufferAllocator() = delete;
	~SendBufferAllocator() = default;

	SendBufferAllocator(const SendBufferAllocator&) = delete;
	SendBufferAllocator& operator=(const SendBufferAllocator&) = delete;
	SendBufferAllocator(SendBufferAllocator&&) = delete;
	SendBufferAllocator& operator=(SendBufferAllocator&&) = delete;

	[[nodiscard]]
	static INetBufferRef Alloc(const int32 size);

private:

	[[nodiscard]]
	static int32 getBucketIndex(const int32 size, const int32 stride);

	// 1. 함수 포인터 타입 정의 (매개변수 없이 INetBufferRef를 반환)
	using AllocFunc = INetBufferRef(*)();

	// 2. SharedPtrUtils::Alloc을 호출하는 정적 래퍼 함수
	template <int32 STRIDE, int32 INDEX>
	static INetBufferRef DoAlloc()
	{
		constexpr int32 bufferSize = static_cast<int32>((INDEX + 1) * STRIDE);
		return SharedPtrUtils::Alloc<NetSendBuffer<bufferSize>>();
	}

	// 3. 팩 확장을 이용해 컴파일 타임에 함수 포인터 배열 생성
	template <int32 STRIDE, int32... INDEX>
	static const auto& getTable(std::index_sequence<INDEX...>)
	{
		static const std::array<AllocFunc, sizeof...(INDEX)> table =
		{
			&DoAlloc<STRIDE, INDEX>...
		};

		return table;
	}
};

