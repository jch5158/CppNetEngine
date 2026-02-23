#pragma once
#include <functional>

class Job
{
public:
	using CallbackType = std::function<void()>;

	explicit Job(CallbackType&& func)
		: mJobFunc(std::move(func))
	{
	}

	template<typename T, typename Ret, typename... Args>
	Job(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		mJobFunc = [owner, memFunc, ...capArgs = std::forward<Args>(args)]()->void
			{
				(owner.get()->*memFunc)(std::forward<Args>(capArgs)...);
			};
	}

	void Execute() const
	{
		mJobFunc();
	}

private:

	CallbackType mJobFunc;
};

