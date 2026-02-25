#include "pch.h"
#include "JobQueue.h"
#include "JobScheduler.h"

JobQueue::JobQueue()
	: mJobQueue()
	, mIsExecuting(false)
{
}

void JobQueue::Push(const JobRef& pJob, const JobSchedulerRef& pScheduler)
{
	if (mJobQueue.TryEnqueue(pJob) == false)
	{
		ASSERT(false, "JobQueue::Push - Job queue is full. Failed to push job.");
		return;
	}

	pScheduler->Push(shared_from_this());
}

void JobQueue::Execute(const JobTimeBudget& jobTimeBudget)
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

void JobQueue::Flush()
{
	JobRef pJob;
	while (mJobQueue.TryDequeue(pJob))
	{
		pJob->Execute();
	}
}

void JobQueue::Clear()
{
	mJobQueue.Clear();
}

int32 JobQueue::Count() const
{
	return mJobQueue.Count();
}
