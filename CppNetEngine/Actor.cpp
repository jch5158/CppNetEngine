#include "pch.h"
#include "Actor.h"
#include "JobScheduler.h"

Actor::Actor()
	: mSeed(sSeedBase.fetch_add(1))
	, mbAcquire(false)
	, mJobQueue()
{
}

void Actor::Push(const JobRef& pJob, const JobSchedulerRef& pScheduler)
{
	if (mJobQueue.TryEnqueue(pJob) == false)
	{
		NET_ENGINE_LOG_ERROR("JobQueue::Push - mJobQueue.TryEnqueue is failed, mJobQueue.Count() : {}", mJobQueue.Count());
		return;
	}

	pScheduler->Push(shared_from_this());
}

void Actor::Execute(const JobTimeBudget& jobTimeBudget)
{
	if (mbAcquire.exchange(true) == true)
	{
		return;
	}

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
	}
	while (!jobTimeBudget.IsExpired());

	mbAcquire.store(false);
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

std::atomic<int64> Actor::sSeedBase = 0;
