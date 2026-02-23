#pragma once
#include "JobQueue.h"

class TimingJob
{
public:
	explicit TimingJob(JobRef pJob, const JobQueueRef& pOwnerQueue);
	TimingJob(TimingJob&&) = default;
	TimingJob& operator=(TimingJob&&) = default;

	void Execute() const;

private:
	JobRef mJob;
	JobQueueWeak mOwnerQueue;
};

class JobTimingWheel
{
public:

	JobTimingWheel(const JobTimingWheel&) = delete;
	JobTimingWheel& operator=(const JobTimingWheel&) = delete;
	JobTimingWheel(JobTimingWheel&&) = delete;
	JobTimingWheel& operator=(JobTimingWheel&&) = delete;

	explicit JobTimingWheel(const int64 tickIntervalMs, const int32 wheelSize);
	~JobTimingWheel() = default;

	void Reserve(const JobRef& pJob, const JobQueueRef& pOwnerQueue, const int64 delayMs);
	void Tick();

private:

	const int64 mTickIntervalMs;
	const int32 mWheelSize;
	int32 mCurrentSlotIndex;
	std::chrono::steady_clock::time_point mLastTickTime;

	Mutex mWheelMutex;
	Vector<Vector<TimingJob>> mWheel;
};

