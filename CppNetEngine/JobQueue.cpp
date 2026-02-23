#include "pch.h"
#include "JobQueue.h"
#include "JobScheduler.h"

JobQueue::JobQueue()
	: mJobQueue()
	, mIsExecuting(false)
{
}

void JobQueue::Push(const JobRef& pJob)
{
	if (mJobQueue.TryEnqueue(pJob) == false)
	{
		ASSERT(false, "JobQueue::Push - Job queue is full. Failed to push job.");
		return;
	}

	JobScheduler::GetInstance().Push(shared_from_this());
}

void JobQueue::Execute(const JobTimeBudget& jobTimeBudget)
{
	if (mIsExecuting.exchange(true) == true)
	{
		return;
	}
	
	spTlsJobQueue = shared_from_this();

	while (!jobTimeBudget.IsExpired())
	{
		JobRef pJob;
		if (mJobQueue.TryDequeue(pJob) == false)
		{
			break;
		}

		pJob->Execute();
	}
	
	mIsExecuting.store(false);
}

void JobQueue::Clear()
{
	mJobQueue.Clear();
}

int32 JobQueue::Count() const
{
	return mJobQueue.Count();
}
