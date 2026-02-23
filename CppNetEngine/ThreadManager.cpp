#include "pch.h"
#include "ThreadManager.h"

#include "JobQueue.h"
#include "JobScheduler.h"

ThreadManager::ThreadManager()
	: ISingleton<ThreadManager>()
	, mLock()
	, mThreads()
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
	for (auto& thread : mThreads)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}

	mThreads.clear();
}

void ThreadManager::InitTls()
{
	static std::atomic<uint32> sThreadId = 1;
	sTlsThreadId = sThreadId.fetch_add(1);
}

void ThreadManager::DestroyTls()
{
	sTlsThreadId = 0;
}

//void ThreadManager::ExecuteJob()
//{
//	while (true)
//	{
//		auto now = std::chrono::steady_clock::now();
//
//		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - sTlsJobWorkEndTime).count() < DEFAULT_JOB_WORK_DURATION)
//		{
//			const JobQueueRef pJobQueue = JobScheduler::GetInstance().Pop();
//			pJobQueue->Execute();
//		}
//		else
//		{
//			sTlsJobWorkEndTime = now;
//			break;
//		}
//	}
//}

uint32 ThreadManager::GetThreadId()
{
	if (sTlsThreadId == 0)
	{
		InitTls();
	}

	return sTlsThreadId;
}

thread_local uint32 ThreadManager::sTlsThreadId = 0;
