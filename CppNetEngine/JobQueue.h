#pragma once
#include "LockFreeQueue.h"
#include "Job.h"
#include "JobScheduler.h"

class JobQueue : public std::enable_shared_from_this<JobQueue>
{
public:

	using CallbackType = std::function<void()>;

	explicit JobQueue();

	void DoAsync(const JobSchedulerRef& pScheduler, CallbackType&& callback)
	{
		Push(cpp_net_engine::MakeShared<Job>(std::move(callback)), pScheduler);
	}

	template<typename T, typename Ret, typename... Args>
	void DoAsync(const JobSchedulerRef& pScheduler, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		auto pOwner = static_pointer_cast<T>(shared_from_this());
		Push(cpp_net_engine::MakeShared<Job>(pOwner, memFunc, std::forward<Args>(args)...), pScheduler);
	}

	void DoTimer(const JobSchedulerRef& pScheduler, const int64 delayMs, CallbackType&& callback)
	{
		const JobRef pJob = cpp_net_engine::MakeShared<Job>(std::move(callback));
		pScheduler->Reserve(pJob, shared_from_this(), delayMs);
	}

	template<typename T, typename Ret, typename... Args>
	void DoTimer(const JobSchedulerRef& pScheduler, const int64 delayMs, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		auto pOwner = static_pointer_cast<T>(shared_from_this());
		const JobRef pJob = cpp_net_engine::MakeShared<Job>(pOwner, memFunc, std::forward<Args>(args)...);
		pScheduler->Reserve(pJob, shared_from_this(), delayMs);
	}

	void Push(const JobRef& pJob, const JobSchedulerRef& pScheduler);
	void Execute(const JobTimeBudget& jobTimeBudget);
	void Flush();
	void Clear();
	[[nodiscard]] int32 Count() const;

private:
	LockFreeQueue<JobRef> mJobQueue;
	std::atomic<bool> mIsExecuting;
};
