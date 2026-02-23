#pragma once
#include <functional>
#include "LockFreeQueue.h"


class JobTimeBudget
{
public:
	static constexpr int64 DEFAULT_TIME_SLICE_MS = 16;
	
	explicit JobTimeBudget(const std::chrono::milliseconds timeSlice)
		: mTimeSlice(timeSlice)
		, mStart(std::chrono::steady_clock::now())
	{
	}

	[[nodiscard]] 
	bool IsExpired() const
	{
		const auto now = std::chrono::steady_clock::now();
		return (now - mStart) >= mTimeSlice;
	}

private:
	const std::chrono::milliseconds mTimeSlice;
	const std::chrono::steady_clock::time_point mStart;
};

class JobScheduler : public ISingleton<JobScheduler>
{
public:

	JobScheduler();
	virtual ~JobScheduler() override;

	void Push(const JobQueueRef& pJobQueue);
	void Dispatch();

private:

	HANDLE mJobIocpHandle;
	LockFreeQueue<JobQueueRef> mDispatchQueue;
};

