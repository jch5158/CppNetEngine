#pragma once
class JobDispatcher
{
public:

	JobDispatcher() = delete;
	~JobDispatcher() = delete;

	static void Post(const JobRef& pJob, const IActorRef& pActor, const ActorSchedulerRef& pScheduler);
	static void PostDelay(const JobRef& pJob, const IActorRef& pActor, const ActorSchedulerRef& pScheduler, const int64 delayMs);
};

