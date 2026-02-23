#pragma once
#include "LockFreeQueue.h"
#include "Job.h"

class JobQueue : public std::enable_shared_from_this<JobQueue>
{
public:

	using CallbackType = std::function<void()>;

	explicit JobQueue();

	void DoAsync(CallbackType&& callback)
	{
		Push(cpp_net_engine::MakeShared<Job>(std::move(callback)));
	}

	template<typename T, typename Ret, typename... Args>
	void DoAsync(Ret(T::* memFunc)(Args...), Args... args)
	{
		std::shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		Push(cpp_net_engine::MakeShared<Job>(owner, memFunc, std::forward<Args>(args)...));
	}

	void Push(const JobRef& pJob);
	void Execute();
	void Clear();

private:
	static thread_local JobQueueRef spTlsJobQueue;

	LockFreeQueue<JobRef> mJobQueue;
	std::atomic<bool> mIsExecuting;
};
