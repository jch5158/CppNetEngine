#include "pch.h"
#include "JobQueue.h"
#include "JobScheduler.h"

JobScheduler::JobScheduler()
	:mJobIocpHandle(nullptr)
	, mTimingWheel(JobTimingWheel(TICK_INTERVAL_MS, WHEEL_SIZE))
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
	if (mJobIocpHandle != nullptr)
	{
		CloseHandle(mJobIocpHandle);
	}
}

void JobScheduler::Push(const JobQueueRef& pJobQueue)
{
	if (mDispatchQueue.TryEnqueue(pJobQueue) == false)
	{
		ASSERT(false, "JobScheduler::Push - Job queue manager is full. Failed to push job queue.");
		return;
	}

	if (PostQueuedCompletionStatus(mJobIocpHandle, 0, 0, nullptr) == false)
	{
		ASSERT(false, "JobScheduler::Push - PostQueuedCompletionStatus is Failed");
	}
}

void JobScheduler::Dispatch()
{
	DWORD bytesTransferred = 0;
	ULONG_PTR pCompletionKey = 0;
	OVERLAPPED* pOverlapped = nullptr;

	if (!GetQueuedCompletionStatus(mJobIocpHandle, &bytesTransferred, &pCompletionKey, &pOverlapped, INFINITE))
	{
		return;
	}

	const auto jobTimeBudget = JobTimeBudget(std::chrono::milliseconds(JobTimeBudget::DEFAULT_TIME_SLICE_MS));
	while (!jobTimeBudget.IsExpired())
	{
		JobQueueRef pJobQueue = nullptr;
		if (!mDispatchQueue.TryDequeue(pJobQueue))
		{
			break;
		}

		pJobQueue->Execute(jobTimeBudget);

		if (pJobQueue->Count() > 0)
		{
			Push(pJobQueue);
		}
	}
}

void JobScheduler::Reserve(const JobRef& pJob, const JobQueueRef& pOwnerQueue, const int64 delayMs)
{
	mTimingWheel.Reserve(pJob, pOwnerQueue, delayMs);
}

void JobScheduler::Tick()
{
	mTimingWheel.Tick();
}
