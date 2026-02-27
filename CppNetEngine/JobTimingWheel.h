#pragma once

class TimingJob
{
public:
	explicit TimingJob(JobRef pJob, const ActorRef& pOwner, JobSchedulerRef pScheduler);
	TimingJob(TimingJob&&) = default;
	TimingJob& operator=(TimingJob&&) = default;

	void Execute() const;

private:
	JobRef mJob;
	ActorWeak mpOwnerQueue;
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

	void Reserve(const JobRef& pJob, const ActorRef& pOwner, const JobSchedulerRef& pScheduler, const int64 delayMs);
	void Tick();

private:

	class Ticking
	{
	public:
		explicit Ticking(bool &isTicking)
			:mIsTicking(isTicking)
		{
		}

		[[nodiscard]] bool IsTicking() const
		{
			return mIsTicking.exchange(true) != true;
		}

		~Ticking()
		{
			mIsTicking.store(false);
		}

	private:
		std::atomic_ref<bool> mIsTicking;
	};

	const int64 mTickIntervalMs;
	const int32 mWheelSize;
	std::chrono::steady_clock::time_point mLastTickTime;

	Mutex mWheelMutex;
	bool mIsTicking;
	int32 mCurrentSlotIndex;
	Vector<Vector<TimingJob>> mWheel;
};

