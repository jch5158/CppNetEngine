#pragma once
#include "LockFreeQueue.h"
#include "Job.h"
#include "JobScheduler.h"

class IActor : public std::enable_shared_from_this<Actor>
{
public:

	using CallbackType = std::function<void()>;

	explicit IActor();
	virtual ~IActor() = default;

	virtual void DoAsync(const JobSchedulerRef& pScheduler, CallbackType&& callback) = 0;
	virtual void DoTimer(const JobSchedulerRef& pScheduler, const int64 delayMs, CallbackType&& callback) = 0;
	virtual void Execute(const JobTimeBudget& jobTimeBudget) = 0;
	virtual void Register(const JobSchedulerRef& pScheduler);
	virtual bool TryAcquire() = 0;
	virtual void Release() = 0;
	virtual void Push(const JobRef& pJob, const JobSchedulerRef& pScheduler) = 0;
	virtual void Flush() = 0;
};

class Actor : public IActor
{
public:

	explicit Actor();
	virtual ~Actor() override = default;

	virtual void DoAsync(const JobSchedulerRef& pScheduler, CallbackType&& callback) override
	{
		Push(cpp_net_engine::MakeShared<Job>(std::move(callback)), pScheduler);
	}

	template<typename T, typename Ret, typename... Args>
	void DoAsync(const JobSchedulerRef& pScheduler, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		auto pOwner = static_pointer_cast<T>(shared_from_this());
		Push(cpp_net_engine::MakeShared<Job>(pOwner, memFunc, std::forward<Args>(args)...), pScheduler);
	}

	virtual void DoTimer(const JobSchedulerRef& pScheduler, const int64 delayMs, CallbackType&& callback) override
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

	virtual void Execute(const JobTimeBudget& jobTimeBudget) override;
	virtual void Register(const JobSchedulerRef& pScheduler) override;
	virtual bool TryAcquire() override;
	virtual void Release() override;
	virtual void Push(const JobRef& pJob, const JobSchedulerRef& pScheduler) override;
	virtual void Flush() override;

	void Clear();
	[[nodiscard]] int32 Count() const;
	[[nodiscard]] int64 GetSeed() const;

private:
	static std::atomic<int64> sSeedBase;

	const int64 mSeed;
	std::atomic<bool> mbAcquire;
	LockFreeQueue<JobRef> mJobQueue;
};
