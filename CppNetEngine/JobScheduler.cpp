#include "pch.h"
#include "JobScheduler.h"
#include "JobQueue.h"

JobScheduler::JobScheduler()
	:mJobIocpHandle(INVALID_HANDLE_VALUE)
	, mDispatchQueue()
{
	mJobIocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (mJobIocpHandle == nullptr)
	{
		CrashReporter::Crash();
	}
}

JobScheduler::~JobScheduler()
{
	if (mJobIocpHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(mJobIocpHandle);
	}
}

void JobScheduler::Push(const JobQueueRef& pJobQueue)
{
	if (mDispatchQueue.TryEnqueue(pJobQueue) == false)
	{
		ASSERT(false, "JobScheduler::Push - Job queue manager is full. Failed to push job queue.");
	}

	if (PostQueuedCompletionStatus(mJobIocpHandle, 0, 0, nullptr) == false)
	{
		ASSERT(false, "JobScheduler::Push - PostQueuedCompletionStatus is Failed");
	}
}

void JobScheduler::Dispatch()
{
	DWORD bytesTransferred = 0;
	ULONG_PTR completionKey = 0;
	LPOVERLAPPED overlapped = nullptr;

	if (!GetQueuedCompletionStatus(mJobIocpHandle, &bytesTransferred, &completionKey, &overlapped, INFINITE))
	{
		return;
	}

	const auto jobTimeBudget = JobTimeBudget(std::chrono::milliseconds(JobTimeBudget::DEFAULT_TIME_SLICE_MS));
	if (!jobTimeBudget.IsExpired())
	{
		JobQueueRef pJobQueue = nullptr;
		if (!mDispatchQueue.TryDequeue(pJobQueue))
		{
			return;
		}

		pJobQueue->Execute();
	}
}
