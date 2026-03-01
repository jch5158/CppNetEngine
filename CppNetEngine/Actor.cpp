#include "pch.h"
#include "Actor.h"
#include "JobScheduler.h"

Actor::Actor()
	: mSeed(sSeedBase.fetch_add(1))
	, mbAcquire(false)
	, mJobQueue()
{
}

void Actor::Execute(const JobTimeBudget& jobTimeBudget)
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

void Actor::Register(const JobSchedulerRef& pScheduler)
{
	if (mJobQueue.Count() > 0)
	{
		pScheduler->Push(shared_from_this());
	}
}

bool Actor::TryAcquire()
{
	if (mbAcquire.exchange(true) == true)
	{
		return false;
	}

	return true;
}

void Actor::Release()
{
	mbAcquire.store(false);
}

void Actor::Push(const JobRef& pJob, const JobSchedulerRef& pScheduler)
{
	if (mJobQueue.TryEnqueue(pJob) == false)
	{
		NET_ENGINE_LOG_ERROR("Actor::Push - mJobQueue.TryEnqueue is failed, mJobQueue.Count() : {}", mJobQueue.Count());
		return;
	}

	Register(pScheduler);
}

void Actor::Flush()
{
	JobRef pJob;
	while (mJobQueue.TryDequeue(pJob))
	{
		pJob->Execute();
	}
}

void Actor::Clear()
{
	mJobQueue.Clear();
}

int32 Actor::Count() const
{
	return mJobQueue.Count();
}

int64 Actor::GetSeed() const
{
	return mSeed;
}

std::atomic<int64> Actor::sSeedBase = 0;
