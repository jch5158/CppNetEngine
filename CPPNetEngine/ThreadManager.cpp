#include "pch.h"
#include "ThreadManager.h"

ThreadManager::ThreadManager()
	: ISingleton<ThreadManager>()
{
	InitTls();
}

ThreadManager::~ThreadManager()
{
	JoinWithClear();
}

void ThreadManager::Launch(const std::function<void()>& callback)
{
	LockGuard guard(mLock);

	mThreads.emplace_back([&]()->void
	{
		ThreadManager::InitTls();
		callback();
		ThreadManager::DestroyTls();
	});
}

void ThreadManager::JoinWithClear()
{
	for (auto& t : mThreads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}

	mThreads.clear();
}

void ThreadManager::InitTls()
{
	static std::atomic<int32> sThreadId = 0;
	net_engine_tls::gThreadId = ++sThreadId;
}

void ThreadManager::DestroyTls()
{
}
