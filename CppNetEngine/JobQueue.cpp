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

void JobQueue::Execute()
{
	if (mIsExecuting.exchange(true) == true)
	{
		return;
	}
	
	spTlsJobQueue = shared_from_this();

	const int32 jobCount = mJobQueue.Count();
	for (int32 i = 0; i < jobCount; ++i)
	{
		JobRef pJob;
		if (mJobQueue.TryDequeue(pJob) == false)
		{
			ASSERT(false, "JobQueue::Execute - Failed to dequeue job.");
			continue;
		}

		pJob->Execute();
	}
	
	spTlsJobQueue = nullptr;

	mIsExecuting.store(false);
}

void JobQueue::Clear()
{
	mJobQueue.Clear();
}