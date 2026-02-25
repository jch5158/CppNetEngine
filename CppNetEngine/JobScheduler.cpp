#include "pch.h"
#include "JobQueue.h"
#include "JobScheduler.h"

JobScheduler::JobScheduler()
	: mJobIocpHandle(nullptr)
	, mTimingWheel(JobTimingWheel(TICK_INTERVAL_MS, WHEEL_SIZE))
	, mDispatchQueue()
	, mpOnErrorHandler([](const uint32)->void {})
{
	mJobIocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (mJobIocpHandle == nullptr)
	{
		CrashReporter::Crash();
	}
}

JobScheduler::JobScheduler(std::function<void(const uint32)> pOnErrorHandler)
	: mJobIocpHandle(nullptr)
	, mTimingWheel(JobTimingWheel(TICK_INTERVAL_MS, WHEEL_SIZE))
	, mDispatchQueue()
	, mpOnErrorHandler(std::move(pOnErrorHandler))
{
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
		NET_ENGINE_LOG_ERROR("JobScheduler::Push - mDispatchQueue.TryEnqueue is failed, mDispatchQueue.Count() : {}", mDispatchQueue.Count());
		return;
	}

	if (PostQueuedCompletionStatus(mJobIocpHandle, 0, 0, nullptr) == false)
	{
		NET_ENGINE_LOG_ERROR("JobScheduler::Push - PostQueuedCompletionStatus is Failed, errorCode : {}", GetLastError());
	}
}

void JobScheduler::Dispatch()
{
	DWORD bytesTransferred = 0;
	ULONG_PTR pCompletionKey = 0;
	OVERLAPPED* pOverlapped = nullptr;

	const auto jobTimeBudget = JobTimeBudget(TIME_SLICE_MS);
	const int32 gqcsRet = GetQueuedCompletionStatus(mJobIocpHandle, &bytesTransferred, &pCompletionKey, &pOverlapped, static_cast<DWORD>(jobTimeBudget.RemainingTimeMs()));
	if (gqcsRet != 0)
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
	else
	{
		const uint32 errorCode = GetLastError();
		if (errorCode != WAIT_TIMEOUT)
		{
			mpOnErrorHandler(errorCode);
			return;
		}
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
