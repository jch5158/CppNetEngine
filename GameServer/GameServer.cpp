#include "pch.h"

#include "GameSession.h"
#include "IocpCore.h"
#include "Service.h"
#include "JobScheduler.h"
#include "ThreadManager.h"

int main()
{
	NetEngineInit netEngineInit;

	CrashReporter::Init(L"GameServer", L"1.0.0", L"");

	const ServerServiceRef pService = cpp_net_engine::MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		cpp_net_engine::MakeShared<IocpCore>(),
		cpp_net_engine::MakeShared<JobScheduler>(),
		cpp_net_engine::MakeShared<GameSession>,
		1);

	pService->Start();

	NET_ENGINE_LOG_INFO("GameServer is started.");

	for (int32 i = 0; i < 5; ++i)
	{
		ThreadManager::GetInstance().Launch([pService]()->void
			{
				while (true)
				{
					pService->GetIocpCore()->Dispatch();
				}
			});

		ThreadManager::GetInstance().Launch([pService]()->void
			{
				while (true)
				{
					pService->GetJobScheduler()->Dispatch();
				}
			});
	}

	ThreadManager::GetInstance().JoinWithClear();
}

