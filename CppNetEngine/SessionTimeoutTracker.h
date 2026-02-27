#pragma once

class SessionTimeoutTracker
{
public:

	static constexpr int64 DEFAULT_TIME_OUT = 60000;

	SessionTimeoutTracker(const SessionTimeoutTracker&) = delete;
	SessionTimeoutTracker operator=(const SessionTimeoutTracker&) = delete;
	SessionTimeoutTracker(SessionTimeoutTracker&&) = delete;
	SessionTimeoutTracker operator=(SessionTimeoutTracker&&) = delete;

	explicit SessionTimeoutTracker(const int64 timeoutMs = DEFAULT_TIME_OUT);
	~SessionTimeoutTracker() = default;

	void UpdateActivity();
	[[nodiscard]] bool IsExpired() const;

private:
	
	static int64 getNowTimeMs();

	const int64 mTimeoutMs;
	std::atomic<int64> mLastActivityMs;
};

