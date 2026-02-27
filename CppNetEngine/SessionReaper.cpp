#include "pch.h"
#include "SessionReaper.h"
#include "Session.h"


// ReSharper disable once CppMemberFunctionMayBeStatic
void SessionReaper::ReapSession(const WeakSessionRef& pWeakSession)
{
	const SessionRef pSession = pWeakSession.lock();
	if (pSession == nullptr)
	{
		return;
	}

	if (pSession->OnIsExpired())
	{
		pSession->Disconnect(eDisconnectReason::Timeout);
	}
	else
	{
		pSession->RegisterReapSelf();
	}
}
