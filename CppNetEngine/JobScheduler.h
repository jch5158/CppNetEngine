#pragma once
#include <functional>

#include "JobTimingWheel.h"
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

	static constexpr int64 TICK_INTERVAL_MS = 10;
	static constexpr int32 WHEEL_SIZE = 6000;

	JobScheduler();
	virtual ~JobScheduler() override;

	void Push(const JobQueueRef& pJobQueue);
	void Dispatch();
	void Reserve(const JobRef& pJob, const JobQueueRef& pOwnerQueue, const int64 delayMs);
	void Tick();

private:

	HANDLE mJobIocpHandle;
	JobTimingWheel mTimingWheel;
	LockFreeQueue<JobQueueRef> mDispatchQueue;
};

