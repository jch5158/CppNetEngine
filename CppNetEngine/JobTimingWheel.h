#pragma once

class TimingJob
{
public:
	explicit TimingJob(JobRef pJob, const JobQueueRef& pOwnerQueue, JobSchedulerRef pScheduler);
	TimingJob(TimingJob&&) = default;
	TimingJob& operator=(TimingJob&&) = default;

	void Execute() const;

private:
	JobRef mJob;
	JobQueueWeak mpOwnerQueue;
	JobSchedulerRef mpScheduler;
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

	void Reserve(const JobRef& pJob, const JobQueueRef& pOwnerQueue, const JobSchedulerRef& pScheduler, const int64 delayMs);
	void Tick();

private:

	const int64 mTickIntervalMs;
	const int32 mWheelSize;
	std::chrono::steady_clock::time_point mLastTickTime;

	Mutex mWheelMutex;
	std::atomic<bool> mIsTicking;
	int32 mCurrentSlotIndex;
	Vector<Vector<TimingJob>> mWheel;
};

