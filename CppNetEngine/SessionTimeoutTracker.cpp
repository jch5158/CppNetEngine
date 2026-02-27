#include "pch.h"
#include "SessionTimeoutTracker.h"

SessionTimeoutTracker::SessionTimeoutTracker(const int64 timeoutMs)
	: mTimeoutMs(timeoutMs)
	, mLastActivityMs(getNowTimeMs())
{
}

void SessionTimeoutTracker::UpdateActivity()
{
	const auto now = getNowTimeMs();
	if (now - mLastActivityMs.load() > 1000)
	{
		mLastActivityMs.store(getNowTimeMs());
	}
}

bool SessionTimeoutTracker::IsExpired() const
{
	const auto now = getNowTimeMs();
	if (now - mLastActivityMs.load() > mTimeoutMs)
	{
		return true;
	}

	return false;
}

int64 SessionTimeoutTracker::GetLastActivityMs() const
{
	return mLastActivityMs.load();
}

int64 SessionTimeoutTracker::GetTimeoutMs() const
{
	return mTimeoutMs;
}

int64 SessionTimeoutTracker::getNowTimeMs()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
