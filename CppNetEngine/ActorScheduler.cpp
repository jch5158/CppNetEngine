#include "pch.h"
#include "ActorScheduler.h"
#include "Actor.h"

ActorScheduler::ActorScheduler(std::function<void(const uint32)> pOnHandleError,
	const uint32 timeSliceMs,
	const int32 executeJobCount,
	const int64 tickIntervalMs,
	const int32 wheelSize)
	: mJobIocpHandle(nullptr)
	, mTimeSliceMs(timeSliceMs)
	, mExecuteJobCount(executeJobCount)
	, mTimingWheel(JobTimingWheel(tickIntervalMs, wheelSize))
	, mpOnHandleError(std::move(pOnHandleError))
{
	mJobIocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (mJobIocpHandle == nullptr)
	{
		CrashReporter::Crash();
	}
}

ActorScheduler::~ActorScheduler()
{
	if (mJobIocpHandle != nullptr)
	{
		CloseHandle(mJobIocpHandle);
	}
}

void ActorScheduler::Schedule(const IActorRef& pActor, const bool bBypassAcquire) const
{
	auto& overlapped = pActor->GetActorOverlapped();
	overlapped.ClearOverlapped();
	overlapped.SetOwner(pActor);

	if (bBypassAcquire || pActor->TryAcquire())
	{
		if (PostQueuedCompletionStatus(mJobIocpHandle, 0, 0, &overlapped) == false)
		{
			NET_ENGINE_LOG_ERROR("ActorScheduler::Push - PostQueuedCompletionStatus is Failed, errorCode : {}", GetLastError());
		}

		if (!bBypassAcquire)
		{
			pActor->Release();
		}
	}
}

void ActorScheduler::ScheduleDelay(const JobRef& pJob, const IActorRef& pOwner, const int64 delayMs)
{
	mTimingWheel.Reserve(pJob, pOwner, shared_from_this(), delayMs);
}

void ActorScheduler::Dispatch()
{
	DWORD bytesTransferred = 0;
	ULONG_PTR pCompletionKey = 0;
	ActorOverlapped* pActorOverlapped = nullptr;

	const int32 gqcsRet = GetQueuedCompletionStatus(mJobIocpHandle, &bytesTransferred, &pCompletionKey, reinterpret_cast<LPOVERLAPPED*>(&pActorOverlapped), mTimeSliceMs);
	if (gqcsRet == 0)
	{
		const uint32 errorCode = GetLastError();
		if (errorCode != WAIT_TIMEOUT)
		{
			mpOnHandleError(errorCode);
		}
	}

	if (pActorOverlapped != nullptr)
	{
		const IActorRef pActor = pActorOverlapped->GetOwner();
		if (pActor != nullptr && pActor->TryAcquire())
		{
			const int32 currentJobCount = pActor->GetJobCount();
			const int32 executeJobCount = mExecuteJobCount < currentJobCount ? mExecuteJobCount : currentJobCount;

			for (int32 i = 0; i < executeJobCount; ++i)
			{
				pActor->Execute();
			}

			pActor->ClearActorOverlapped();

			pActor->Release();

			pActor->Register(shared_from_this());
		}
	}

	mTimingWheel.Tick();
}
