#pragma once
#include "LockFreeStack.h"
#include "SessionReaper.h"

class SessionManager
{
public:
	SessionManager(const SessionManager&) = delete;
	SessionManager operator=(const SessionManager&) = delete;
	SessionManager(SessionManager&&) = delete;
	SessionManager operator=(SessionManager&&) = delete;

	explicit SessionManager(const int32 maxSessionCount);
	~SessionManager() = default;

	bool AddSession(const SessionRef& pSession);
	void ReleaseSession(const SessionRef& pSession);

	int32 GetMaxSessionCount() const;
	int32 GetCurrentSessionCount() const;

private:

	const int32 mMaxSessionCount;
	std::atomic<int32> mCurrentSessionCount;

	Mutex mLock;
	Set<SessionRef> mSessions;
};
