#pragma once
#include <functional>

#include "Job.h"
#include "JobTimingWheel.h"
#include "LockFreeQueue.h"

class JobTimeBudget
{
public:
	explicit JobTimeBudget(const int64 timeSliceMs)
		: mTimeSlice(std::chrono::milliseconds(timeSliceMs))
		, mStart(std::chrono::steady_clock::now())
	{
	}

	[[nodiscard]] 
	bool IsExpired() const
	{
		const auto now = std::chrono::steady_clock::now();
		return (now - mStart) >= mTimeSlice;
	}

	[[nodiscard]]
	int64 RemainingTimeMs() const
	{
		const auto now = std::chrono::steady_clock::now();
		const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - mStart);
		const auto remainingTime = mTimeSlice - elapsedTime;
		return remainingTime.count();
	}

private:
	const std::chrono::milliseconds mTimeSlice;
	const std::chrono::steady_clock::time_point mStart;
};

class JobScheduler : public std::enable_shared_from_this<JobScheduler>
{
public:

	static constexpr int64 TIME_SLICE_MS = 16;
	static constexpr int64 TICK_INTERVAL_MS = 10;
	static constexpr int32 WHEEL_SIZE = 60 * 60 * 100 * TICK_INTERVAL_MS;

	JobScheduler(const JobScheduler&) = delete;
	JobScheduler& operator=(const JobScheduler&) = delete;
	JobScheduler(JobScheduler&&) = delete;
	JobScheduler& operator=(JobScheduler&&) = delete;

	explicit JobScheduler();
	explicit JobScheduler(std::function<void(const uint32)> pOnHandleError);
	~JobScheduler();

	void Push(const ActorRef& pActor);
	void Dispatch();
	void Reserve(const JobRef& pJob, const ActorRef& pOwner, const int64 delayMs);
	void Flush();
	
private:

	HANDLE mJobIocpHandle;
	JobTimingWheel mTimingWheel;
	LockFreeQueue<ActorRef> mActorQueue;
	std::function<void(const uint32)> mpOnHandleError;
};