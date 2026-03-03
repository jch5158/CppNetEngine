#include "pch.h"
#include "JobDispatcher.h"

void JobDispatcher::Post(const JobRef& pJob, const IActorRef& pActor, const ActorSchedulerRef& pScheduler)
{
	if (pActor->PushJob(pJob) == false)
	{
		NET_ENGINE_LOG_FATAL("JobDispatcher::Post - pActor->PushJob(pJob) is failed, jobCount : {}", pActor->GetJobCount());
		return;
	}

	pActor->Register(pScheduler);
}

void JobDispatcher::PostDelay(const JobRef& pJob, const IActorRef& pActor, const ActorSchedulerRef& pScheduler, const int64 delayMs)
{
	pScheduler->ScheduleDelay(pJob, pActor, delayMs);
}
