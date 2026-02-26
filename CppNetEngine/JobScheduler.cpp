#include "pch.h"
#include "Actor.h"
#include "JobScheduler.h"

JobScheduler::JobScheduler()
	: mJobIocpHandle(nullptr)
	, mTimingWheel(JobTimingWheel(TICK_INTERVAL_MS, WHEEL_SIZE))
	, mActorQueue()
	, mpOnHandleError([](const uint32)->void {})
{
	mJobIocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (mJobIocpHandle == nullptr)
	{
		CrashReporter::Crash();
	}
}

JobScheduler::JobScheduler(std::function<void(const uint32)> pOnHandleError)
	: mJobIocpHandle(nullptr)
	, mTimingWheel(JobTimingWheel(TICK_INTERVAL_MS, WHEEL_SIZE))
	, mActorQueue()
	, mpOnHandleError(std::move(pOnHandleError))
{
}

JobScheduler::~JobScheduler()
{
	if (mJobIocpHandle != nullptr)
	{
		CloseHandle(mJobIocpHandle);
	}
}

void JobScheduler::Push(const ActorRef& pActor)
{
	if (mActorQueue.TryEnqueue(pActor) == false)
	{
		NET_ENGINE_LOG_ERROR("JobScheduler::Push - mActorQueue.TryEnqueue is failed, mActorQueue.Count() : {}", mActorQueue.Count());
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
			ActorRef pActor = nullptr;
			if (!mActorQueue.TryDequeue(pActor))
			{
				break;
			}

			pActor->Execute(jobTimeBudget);

			if (pActor->Count() > 0)
			{
				Push(pActor);
			}
		} while (!jobTimeBudget.IsExpired());
	}
	else
	{
		const uint32 errorCode = GetLastError();
		if (errorCode != WAIT_TIMEOUT)
		{
			mpOnHandleError(errorCode);
			return;
		}
	}

	mTimingWheel.Tick();
}

void JobScheduler::Reserve(const JobRef& pJob, const ActorRef& pOwner, const int64 delayMs)
{
	mTimingWheel.Reserve(pJob, pOwner, shared_from_this(), delayMs);
}

void JobScheduler::Flush()
{
	ActorRef pActor = nullptr;
	while (mActorQueue.TryDequeue(pActor))
	{
		pActor->Flush();
	}
}
