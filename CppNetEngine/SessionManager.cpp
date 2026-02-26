#include "pch.h"
#include "SessionManager.h"
#include "Session.h"

SessionManager::SessionManager(const int32 maxSessionCount)
	: mMaxSessionCount(maxSessionCount)
	, mCurrentSessionCount(0)
	, mSessions()
{
}

bool SessionManager::AddSession(const SessionRef& pSession)
{
	if (pSession == nullptr)
	{
		return false;
	}

	if (mCurrentSessionCount.fetch_add(1) >= mMaxSessionCount)
	{
		mCurrentSessionCount.fetch_sub(1);
		return false;
	}

	UniqueLock lock(mLock);
	return mSessions.emplace(pSession).second;
}

void SessionManager::ReleaseSession(const SessionRef& pSession)
{
	mCurrentSessionCount.fetch_sub(1);

	UniqueLock lock(mLock);
	mSessions.erase(pSession);
}

int32 SessionManager::GetMaxSessionCount() const
{
	return mMaxSessionCount;
}

int32 SessionManager::GetCurrentSessionCount() const
{
	return mCurrentSessionCount.load();
}
