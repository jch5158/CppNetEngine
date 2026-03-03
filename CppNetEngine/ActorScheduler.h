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

// TODO : ActorScheduler로 변경
class ActorScheduler : public std::enable_shared_from_this<ActorScheduler>
{
public:

	static constexpr int64 TIME_SLICE_MS = 16;
	static constexpr int64 TICK_INTERVAL_MS = 10;
	static constexpr int32 WHEEL_SIZE = 60 * 60 * 100 * TICK_INTERVAL_MS;

	ActorScheduler(const ActorScheduler&) = delete;
	ActorScheduler& operator=(const ActorScheduler&) = delete;
	ActorScheduler(ActorScheduler&&) = delete;
	ActorScheduler& operator=(ActorScheduler&&) = delete;

	explicit ActorScheduler();
	explicit ActorScheduler(std::function<void(const uint32)> pOnHandleError);
	~ActorScheduler();

	void Schedule(const IActorRef& pActor, const bool bBypassAcquire = false) const;
	void ScheduleDelay(const JobRef& pJob, const IActorRef& pOwner, const int64 delayMs);
	void Dispatch();
	
private:

	HANDLE mJobIocpHandle;
	JobTimingWheel mTimingWheel;
	std::function<void(const uint32)> mpOnHandleError;
};