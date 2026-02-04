#pragma once
#include <mutex>

template <typename T>
class StlAllocator;

using byte = unsigned char;
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

using Mutex = std::mutex;
using ConditionVar = std::condition_variable;
using UniqueLock = std::unique_lock<std::mutex>;
using LockGuard = std::scoped_lock<std::mutex>;

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
using PriorityQueue = std::priority_queue<T, Vector<T>, std::less<typename Vector<T>::value_type>>;

template<typename Key, typename Type>
using Map = std::map<Key, Type, std::less<Key>, StlAllocator<std::pair<const Key, Type>>>;

template<typename Key>
using Set = std::set<Key, std::less<Key>, StlAllocator<Key>>;

template<typename Key, typename Type, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
using HashMap = std::unordered_map<Key, Type, Hasher, KeyEq, StlAllocator<std::pair<const Key, Type>>>;

template<typename Key, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
using HashSet = std::unordered_set<Key, Hasher, KeyEq, StlAllocator<Key>>;

using String = std::basic_string<char, std::char_traits<char>, StlAllocator<char>>;

using WString = std::basic_string<wchar_t, std::char_traits<wchar_t>, StlAllocator<wchar_t>>;
