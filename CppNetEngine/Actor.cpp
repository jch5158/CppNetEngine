#include "pch.h"
#include "Actor.h"
#include "JobScheduler.h"

Actor::Actor()
	: mSeed(sSeedBase.fetch_add(1))
	, mJobQueue()
	, mIsExecuting(false)
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
	if (mIsExecuting.exchange(true) == true)
	{
		return;
	}

	do
	{
		JobRef pJob;
		if (mJobQueue.TryDequeue(pJob) == false)
		{
			break;
		}

		pJob->Execute();
	}
	while (!jobTimeBudget.IsExpired());

	mIsExecuting.store(false);
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
