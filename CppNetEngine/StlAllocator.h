#pragma once
#include "MemoryAllocator.h"

template <typename T>
class StlAllocator
{
public:
	using value_type = T;
	
	explicit StlAllocator() = default;
	explicit StlAllocator(const StlAllocator&) = default;
	StlAllocator& operator=(const StlAllocator&) = default;
	explicit StlAllocator(StlAllocator&&) = default;
	StlAllocator& operator=(StlAllocator&&) = default;
	~StlAllocator() = default;

	template <typename U>
	explicit StlAllocator(const StlAllocator<U>&) {}

	// ReSharper disable once CppInconsistentNaming
	static T* allocate(const uint64 size)
	{
		void * pData = MemoryAllocator::GetInstance().Alloc(size * sizeof(T));

		return static_cast<T*>(pData);
	}

	// ReSharper disable once CppInconsistentNaming
	static void deallocate(T* pData, const uint64 size)
	{
		MemoryAllocator::GetInstance().Free(reinterpret_cast<void*>(pData), size * sizeof(T));
	}
};

template <typename T>
using Vector = std::vector<T, StlAllocator<T>>;

template<typename T>
using List = std::list<T, StlAllocator<T>>;

template<typename T>
using Deque = std::deque<T, StlAllocator<T>>;

template<typename T>
using Queue = std::queue<T, Deque<T>>;

template<typename T>
using Stack = std::stack<T, Deque<T>>;

template<typename T>
using PriorityQueue = std::priority_queue<T, Vector<T>, std::less<T>>;

template<typename Key, typename Type>
using Map = std::map<Key, Type, std::less<Key>, StlAllocator<std::pair<const Key, Type>>>;

template<typename Key>
using Set = std::set<Key, std::less<Key>, StlAllocator<Key>>;

template<typename Key, typename Type, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
using HashMap = std::unordered_map<Key, Type, Hasher, KeyEq, StlAllocator<std::pair<const Key, Type>>>;

template<typename Key, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
using HashSet = std::unordered_set<Key, Hasher, KeyEq, StlAllocator<Key>>;

using String = std::basic_string<char, std::char_traits<char>, StlAllocator<char>>;

using Wstring = std::basic_string<wchar_t, std::char_traits<wchar_t>, StlAllocator<wchar_t>>;