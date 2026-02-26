#pragma once
#include "LockFreeStack.h"

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
	void ReleaseSession(const int32 sessionIndex);
	void ReleaseSessionIndex(const int32 sessionIndex);

	int32 GetCurrentSessionCount() const;

private:

	const int32 mMaxSessionCount;
	Vector<SessionRef> mSessions;
	LockFreeStack<int32> mReleaseSessionIndexStack;
};

