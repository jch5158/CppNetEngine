#include "pch.h"
#include "NetEngineGlobal.h"
#include "ThreadManager.h"

void NetEngineGlobal::Initialize()
{
	singletonInit();
}

void NetEngineGlobal::singletonInit()
{
	ThreadManager::GetInstance();
}
