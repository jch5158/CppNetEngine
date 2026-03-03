#include "pch.h"
#include "ActorScheduler.h"

#include "Actor.h"
#include "ActorScheduler.h"

ActorScheduler::ActorScheduler()
	: mJobIocpHandle(nullptr)
	, mTimingWheel(JobTimingWheel(TICK_INTERVAL_MS, WHEEL_SIZE))
	, mpOnHandleError([](const uint32)->void {})
{
	mJobIocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (mJobIocpHandle == nullptr)
	{
		CrashReporter::Crash();
	}
}

ActorScheduler::ActorScheduler(std::function<void(const uint32)> pOnHandleError)
	: mJobIocpHandle(nullptr)
	, mTimingWheel(JobTimingWheel(TICK_INTERVAL_MS, WHEEL_SIZE))
	, mpOnHandleError(std::move(pOnHandleError))
{
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

	const auto timeBudget = ActorTimeBudget(TIME_SLICE_MS);
	const int32 gqcsRet = GetQueuedCompletionStatus(mJobIocpHandle, &bytesTransferred, &pCompletionKey, reinterpret_cast<LPOVERLAPPED*>(&pActorOverlapped), static_cast<DWORD>(timeBudget.RemainingTimeMs()));
	if (gqcsRet != 0)
	{
		do
		{
			const IActorRef pActor = pActorOverlapped->GetOwner();

			if (pActor->TryAcquire())
			{
				pActor->Execute(timeBudget);

				pActor->ClearActorOverlapped();

				pActor->Release();

				pActor->Register(shared_from_this());
			}

		} while (!timeBudget.IsExpired());
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
