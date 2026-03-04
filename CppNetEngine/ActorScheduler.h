#pragma once
#include <functional>

#include "Job.h"
#include "JobTimingWheel.h"
#include "LockFreeQueue.h"

class ActorTimeBudget
{
public:
	explicit ActorTimeBudget(const int64 timeSliceMs)
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

class ActorScheduler : public std::enable_shared_from_this<ActorScheduler>
{
public:

	static constexpr int32 DEFAULT_EXECUTE_JOB_COUNT = 50;
	static constexpr int64 DEFAULT_TIME_SLICE_MS = 16;
	static constexpr int64 DEFAULT_TICK_INTERVAL_MS = 10;
	static constexpr int32 DEFAULT_WHEEL_SIZE = 60 * 60 * 100 * DEFAULT_TICK_INTERVAL_MS;

	ActorScheduler(const ActorScheduler&) = delete;
	ActorScheduler& operator=(const ActorScheduler&) = delete;
	ActorScheduler(ActorScheduler&&) = delete;
	ActorScheduler& operator=(ActorScheduler&&) = delete;

	explicit ActorScheduler(std::function<void(const uint32)> pOnHandleError,
		const uint32 timeSliceMs = DEFAULT_TIME_SLICE_MS,
		const int32 executeJobCount = DEFAULT_EXECUTE_JOB_COUNT,
		const int64 tickIntervalMs = DEFAULT_TICK_INTERVAL_MS,
		const int32 wheelSize = DEFAULT_WHEEL_SIZE);
	virtual ~ActorScheduler();

	void Schedule(const IActorRef& pActor, const bool bBypassAcquire = false) const;
	void ScheduleDelay(const JobRef& pJob, const IActorRef& pOwner, const int64 delayMs);
	void Dispatch();

private:

	HANDLE mJobIocpHandle;
	const uint32 mTimeSliceMs;
	const int32 mExecuteJobCount;
	JobTimingWheel mTimingWheel;
	std::function<void(const uint32)> mpOnHandleError;
};