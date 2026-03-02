#include "pch.h"

#include "GameSession.h"
#include "Service.h"
#include "ThreadManager.h"

#include "Generated/PacketServiceTypeHandler.h"

int main()
{
	CrashReporter::Init(L"GameServer", L"1.0.0", L"");

	NetEngineInit netEngineInit;

	PacketServiceTypeHandler::Init();

	const ServerServiceRef pService = cpp_net_engine::MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		cpp_net_engine::MakeShared<IocpCore>(),
		cpp_net_engine::MakeShared<JobScheduler>(),
		cpp_net_engine::MakeShared<GameSession>,
		cpp_net_engine::MakeShared<SessionManager>(10),
		cpp_net_engine::MakeShared<SessionReaper>(10000),
		cpp_net_engine::MakeShared<WaitQueueManager>(0));

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

