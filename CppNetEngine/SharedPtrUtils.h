// ReSharper disable CppInconsistentNaming
#pragma once

#include "MemoryAllocator.h"

template <typename T>
class SharedPtrAllocator final
{
public:

    using value_type = T;

	explicit SharedPtrAllocator() = default;
	explicit SharedPtrAllocator(const SharedPtrAllocator&) = default;
	explicit SharedPtrAllocator(SharedPtrAllocator&&) = default;
	SharedPtrAllocator& operator=(const SharedPtrAllocator&) = default;
	SharedPtrAllocator& operator=(SharedPtrAllocator&&) = default;
	
	~SharedPtrAllocator() = default;
    
	template <typename U>
	explicit SharedPtrAllocator(const SharedPtrAllocator<U>&) {}

	template<typename U>
	bool operator==(const SharedPtrAllocator<U>&) const { return true; }

	template<typename U>
	bool operator!=(const SharedPtrAllocator<U>&) const { return false; }

    static T* allocate(const uint64 size)
	{
		const uint64 sharedPtrSize = sizeof(T) * size;

        T* ptr = static_cast<T*>(MemoryAllocator::GetInstance().Alloc(sharedPtrSize));

        return ptr;
	}

	static void deallocate(T* ptr, const uint64 size)
	{
        const uint64 objSize = sizeof(T) * size;

        MemoryAllocator::GetInstance().Free(ptr, objSize);
	}
};

class SharedPtrUtils final
{
public:

	SharedPtrUtils() = delete;
	~SharedPtrUtils() = delete;
	SharedPtrUtils(SharedPtrUtils&) = delete;
	SharedPtrUtils& operator=(SharedPtrUtils&) = delete;
	SharedPtrUtils(SharedPtrUtils&&) = delete;
	SharedPtrUtils& operator=(SharedPtrUtils&&) = delete;

	template <typename T, typename... Args>
	static std::shared_ptr<T> Alloc(Args&&... args)
	{
		return std::allocate_shared<T>(SharedPtrAllocator<T>(), std::forward<Args>(args)...);
	}
};

template <typename T>
using WeakPtr = std::weak_ptr<T>;

template <typename T>
using SharedPtr = std::shared_ptr<T>;

using ListenerRef = SharedPtr<class Listen>;
using ListenerConstRef = SharedPtr<const Listen>;

using SessionRef = SharedPtr<class Session>;
using SessionConstRef = SharedPtr<const Session>;

using PacketSessionRef = SharedPtr<class PacketSession>;
using PacketSessionConstRef = SharedPtr<const PacketSession>;

using INetBufferRef = SharedPtr<class INetBuffer>;
using INetBufferConstRef = SharedPtr<const INetBuffer>;

using IocpCoreRef = SharedPtr<class IocpCore>;
using IocpCoreConstRef = SharedPtr<const IocpCore>;

using WeakServiceRef = WeakPtr<class Service>;
using WeakServiceConstRef = WeakPtr<const Service>;
using ServiceRef = SharedPtr<Service>;
using ServiceConstRef = SharedPtr<const Service>;

using IocpObjectRef = SharedPtr<class IocpObject>;
using IocpObjectConstRef = SharedPtr<const IocpObjectRef>;

using ServerServiceRef = SharedPtr<class ServerService>;
using ServerServiceConstRef = SharedPtr<const ServerService>;

using ClientServiceRef = SharedPtr<class ClientService>;
using ClientServiceConstRef = SharedPtr<const ClientService>;

using JobRef = SharedPtr<class Job>;
using JobConstRef = SharedPtr<const Job>;

using JobQueueRef = SharedPtr<class JobQueue>;
using JobQueueConstRef = SharedPtr<const JobQueue>;
using JobQueueWeak = WeakPtr<class JobQueue>;
using JobQueueConstWeak = WeakPtr<const JobQueue>;