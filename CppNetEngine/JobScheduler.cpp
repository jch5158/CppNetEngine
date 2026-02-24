#include "pch.h"
#include "JobQueue.h"
#include "JobScheduler.h"

JobScheduler::JobScheduler()
	: mJobIocpHandle(nullptr)
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

	const auto jobTimeBudget = JobTimeBudget(TIME_SLICE_MS);
	if (!GetQueuedCompletionStatus(mJobIocpHandle, &bytesTransferred, &pCompletionKey, &pOverlapped, static_cast<DWORD>(jobTimeBudget.RemainingTimeMs())))
	{
		const uint32 errorCode = GetLastError();
		if (errorCode != WAIT_TIMEOUT)
		{
			ASSERT(false, "JobScheduler::Dispatch - GetQueuedCompletionStatus is Failed");
			return;
		}
	}
	else
	{
		do
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
		} while (!jobTimeBudget.IsExpired());
	}

	mTimingWheel.Tick();
}

void JobScheduler::Reserve(const JobRef& pJob, const JobQueueRef& pOwnerQueue, const int64 delayMs)
{
	mTimingWheel.Reserve(pJob, pOwnerQueue, shared_from_this(), delayMs);
}

void JobScheduler::Flush()
{
	JobQueueRef pJobQueue = nullptr;
	while (mDispatchQueue.TryDequeue(pJobQueue))
	{
		pJobQueue->Flush();
	}
}
