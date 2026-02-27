#include "pch.h"
#include "SessionReaper.h"
#include "Session.h"

void SessionReaper::ReapSession(const WeakSessionRef& pWeakSession)
{
	const SessionRef pSession = pWeakSession.lock();
	if (pSession == nullptr)
	{
		return;
	}

	if (pSession->OnIsExpired())
	{
		fmt::print(L"Session Reap~!\n");
		pSession->Disconnect(eDisconnectReason::Timeout);
	}
	else
	{
		pSession->RegisterReapSelf();
	}
}
