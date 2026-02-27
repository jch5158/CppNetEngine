#include "pch.h"
#include "SessionReaper.h"
#include "Session.h"

void SessionReaper::ReapSession(const WeakSessionRef& pWeakSession)
{
	SessionRef pSession = pWeakSession.lock();
	if (pSession == nullptr)
	{
		return;
	}

}
