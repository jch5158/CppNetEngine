#pragma once

#include "pch.h"
#include "MemoryAllocator.h"

template <typename T>
class UniquePtrUtils final  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:

	// 삭제자(Deleter)는 T 타입에만 의존해야 합니다. (Args와 무관)
	class Deleter
	{
	public:
		void operator()(T* ptr) const
		{
			if (std::is_class_v<T>)
			{
				ptr->~T();
			}

			MemoryAllocator::GetInstance().Free(static_cast<void*>(ptr), sizeof(T));
		}
	};

	template <typename... Args>
	static std::unique_ptr<T, Deleter> Alloc(Args&&... args)
	{
		const uint64 memSize = sizeof(T);
		void* memPtr = MemoryAllocator::GetInstance().Alloc(memSize);

		T* returnPtr = static_cast<T*>(memPtr);
		if (!std::is_trivially_default_constructible_v<T>)
		{
			returnPtr = new(memPtr) T(std::forward<Args>(args)...);
		}

		return std::unique_ptr<T, Deleter>(returnPtr);
	}
};

// 2. [특수화] 배열용 (Array, 예: char[])
template <typename T>
class UniquePtrUtils<T[]>
{
public:
	// 상태가 있는 Deleter (크기를 기억해야 함)
	class ArrayDeleter
	{
	public:

		explicit ArrayDeleter(const uint64 size)
			:mSize(size)
		{
		}

		void operator()(T* ptr) const
		{
			// 클래스 배열이면 역순 소멸 (char[] 같은 건 무시됨)
			if constexpr (std::is_class_v<T>)
			{
				const uint64 count = mSize / sizeof(T);
				for (uint64 i = 0; i < count; ++i)
				{
					ptr[count - i - 1].~T();
				}
			}

			MemoryAllocator::GetInstance().Free(ptr, mSize);
		}

	private:

		uint64 mSize; // 할당된 바이트 크기 저장
	};

	// 배열은 생성자 인자 전달이 복잡하므로 보통 개수(count)만 받음
	static std::unique_ptr<T[], ArrayDeleter> Alloc(const uint64 count)
	{
		const uint64 size = sizeof(T) * count;
		void* ptr = MemoryAllocator::GetInstance().Alloc(size);

		if constexpr (!std::is_trivially_default_constructible_v<T>)
		{
			for (uint64 i = 0; i < count; ++i)
			{
				new (&ptr[i]) T();
			}
		}

		// ArrayDeleter에 사이즈를 넣어서 생성
		return std::unique_ptr<T[], ArrayDeleter>(static_cast<T*>(ptr), ArrayDeleter(size));
	}
};

template <typename T>
struct GetUniquePtrType { using Type = std::unique_ptr<T, typename UniquePtrUtils<T>::Deleter>; };

template <typename T>
struct GetUniquePtrType<T[]> { using Type = std::unique_ptr<T[], typename UniquePtrUtils<T[]>::ArrayDeleter>; };

template <typename T>
using UniquePtr = GetUniquePtrType<T>::Type;
