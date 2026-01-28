#include "pch.h"
#include "NetEngineGlobal.h"

void net_engine_global::Crash()
{
	volatile uint32* pCrash = nullptr;

#pragma warning(suppress: 6011)
	*pCrash = 0xDEAFBEFF;
}

void net_engine_global::CrashIf(const bool bCrash)
{
	if (bCrash)
	{
		Crash();
	}
}
