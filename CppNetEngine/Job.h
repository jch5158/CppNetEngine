#pragma once
#include <functional>

class Job
{
public:
	using JobFunc = std::function<void()>;

	explicit Job(JobFunc&& func)
		: mJobFunc(std::move(func))
	{
	}

	template<typename T, typename Ret, typename... Args>
	Job(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		mJobFunc = [owner, memFunc, args...]()
			{
				(owner.get()->*memFunc)(args...);
			};
	}

	void Execute() const
	{
		mJobFunc();
	}

private:

	JobFunc mJobFunc;
};

