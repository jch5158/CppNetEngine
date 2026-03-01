#include "pch.h"
#include "ScopedActor.h"

void ScopedActor::DoAsync(const JobSchedulerRef& pScheduler, CallbackType&& callback)
{
	Push(cpp_net_engine::MakeShared<Job>(std::move(callback)), pScheduler);
}

void ScopedActor::DoTimer(const JobSchedulerRef& pScheduler, const int64 delayMs, CallbackType&& callback)
{
	const JobRef pJob = cpp_net_engine::MakeShared<Job>(std::move(callback));
	pScheduler->Reserve(pJob, shared_from_this(), delayMs);
}

void ScopedActor::Execute(const JobTimeBudget& jobTimeBudget)
{
	int32 count = mJobQueue.Count();
	do
	{
		if (count-- <= 0)
		{
			break;
		}

		JobRef pJob;
		if (mJobQueue.TryDequeue(pJob) == false)
		{
			break;
		}

		pJob->Execute();
	} while (!jobTimeBudget.IsExpired());
}

void ScopedActor::Register(const JobSchedulerRef& pScheduler)
{
	if (mJobQueue.Count() > 0)
	{
		pScheduler->Push(shared_from_this());
	}
}

bool ScopedActor::TryAcquire()
{
	if (!tryAcquireAll())
	{
		Release();
		return false;
	}

	return true;
}

void ScopedActor::Release()
{
	for (int32 i = mAcquireIndex; i >= 0; --i)
	{
		mActors[i]->Release();
	}

	mAcquireIndex = -1;
}

void ScopedActor::Push(const JobRef& pJob, const JobSchedulerRef& pScheduler)
{
	if (mJobQueue.TryEnqueue(pJob) == false)
	{
		NET_ENGINE_LOG_ERROR("ScopedActor::Push - mJobQueue.TryEnqueue is failed, mJobQueue.Count() : {}", mJobQueue.Count());
		return;
	}

	Register(pScheduler);
}

void ScopedActor::SetSpinCount(const int32 spinCount)
{
	mSpinCount = spinCount;
}

int32 ScopedActor::GetSpinCount() const
{
	return mSpinCount;
}

bool ScopedActor::tryAcquireAll()
{
	const int32 spinCount = GetSpinCount();
	const int32 actorSize = static_cast<int32>(mActors.size());

	for (int32 i = 0; i < actorSize; ++i)
	{
		for (int32 curSpin = 0; curSpin < spinCount; ++curSpin)
		{
			if (mActors[i]->TryAcquire())
			{
				mAcquireIndex = i;
				break;
			}

			_mm_pause();
		}

		if (mAcquireIndex != i)
		{
			return false;
		}
	}

	return true;
}
