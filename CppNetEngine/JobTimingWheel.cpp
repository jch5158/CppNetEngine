#include "pch.h"
#include "JobTimingWheel.h"
#include "JobQueue.h"

TimingJob::TimingJob(JobRef pJob, const JobQueueRef& pOwnerQueue)
	: mJob(std::move(pJob))
	, mOwnerQueue(pOwnerQueue)
{
}

void TimingJob::Execute() const
{
	if (const auto ownerQueue = mOwnerQueue.lock())
	{
		ownerQueue->Push(mJob);
	}
}


JobTimingWheel::JobTimingWheel(const int64 tickIntervalMs, const int32 wheelSize)
	: mTickIntervalMs(tickIntervalMs)
	, mWheelSize(wheelSize)
	, mCurrentSlotIndex(0)
	, mLastTickTime(std::chrono::steady_clock::now())
	, mWheel(wheelSize)
{
}

void JobTimingWheel::Reserve(const JobRef& pJob, const JobQueueRef& pOwnerQueue, const int64 delayMs)
{
	if (delayMs < 0 || delayMs >= mTickIntervalMs * mWheelSize)
	{
		return;
	}

	int64 ticks = delayMs / mTickIntervalMs;
	if (ticks == 0)
	{
		ticks = 1;
	}

	{
		UniqueLock lock(mWheelMutex);
		
		const int32 slotIndex = static_cast<int32>((mCurrentSlotIndex + ticks) % mWheelSize);

		mWheel[slotIndex].emplace_back(pJob, pOwnerQueue);
	}
}

void JobTimingWheel::Tick()
{
	const auto now = std::chrono::steady_clock::now();
	const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - mLastTickTime).count();
	if (elapsedMs < mTickIntervalMs)
	{
		return;
	}
	
	const int64 processTick = elapsedMs / mTickIntervalMs;
	mLastTickTime += std::chrono::milliseconds(processTick * mTickIntervalMs);

	Vector<TimingJob> expiredJobs;
	{
		UniqueLock lock(mWheelMutex);

		for (int32 i = 0; i < processTick; ++i)
		{
			expiredJobs.insert(expiredJobs.end(), std::make_move_iterator(mWheel[mCurrentSlotIndex].begin()), std::make_move_iterator(mWheel[mCurrentSlotIndex].end()));
			mWheel[mCurrentSlotIndex].clear();
			mCurrentSlotIndex = (mCurrentSlotIndex + 1) % mWheelSize;
		}
	}

	for (const auto& timingJob : expiredJobs)
	{
		timingJob.Execute();
	}
}
