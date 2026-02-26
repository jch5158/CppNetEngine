#include "pch.h"
#include "SessionManager.h"
#include "Session.h"

SessionManager::SessionManager(const int32 maxSessionCount)
	:mMaxSessionCount(maxSessionCount)
	, mSessions(maxSessionCount, nullptr)
	, mReleaseSessionIndexStack(maxSessionCount)
{
	for (int32 i = 0; i < maxSessionCount; ++i)
	{
		if (!mReleaseSessionIndexStack.TryPush(i))
		{
			NET_ENGINE_LOG_FATAL("Service::Service - mReleaseSessionIndexStack is full");
			CrashReporter::Crash();
		}
	}
}

bool SessionManager::AddSession(const SessionRef& pSession)
{
	int32 sessionIndex = -1;
	if (mReleaseSessionIndexStack.TryPop(sessionIndex) == false)
	{
		return false;
	}

	pSession->setSessionIndex(sessionIndex);
	mSessions[sessionIndex] = pSession;

	return true;
}

void SessionManager::ReleaseSession(const int32 sessionIndex)
{
	mSessions[sessionIndex] = nullptr;
}

void SessionManager::ReleaseSessionIndex(const int32 sessionIndex)
{
	if (mReleaseSessionIndexStack.TryPush(sessionIndex) == false)
	{
		NET_ENGINE_LOG_ERROR("SessionManager::ReleaseSessionIndex - TryPush is failed, count : {}", mReleaseSessionIndexStack.Count());
	}
}

int32 SessionManager::GetCurrentSessionCount() const
{
	return mMaxSessionCount - mReleaseSessionIndexStack.Count();
}
