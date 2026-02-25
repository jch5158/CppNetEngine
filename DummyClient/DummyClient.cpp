#include "pch.h"

#include "GameSession.h"
#include "IocpCore.h"
#include "Service.h"
#include "JobScheduler.h"
#include "ThreadManager.h"

int32 main()
{
	NetEngineInit netEngineInit;

	CrashReporter::Init(L"DummyClient", L"1.0.0", L"");

	ClientServiceRef pService = cpp_net_engine::MakeShared<ClientService>(
		NetAddress(L"127.0.0.1", 7777),
		cpp_net_engine::MakeShared<IocpCore>(),
		cpp_net_engine::MakeShared<JobScheduler>(),
		cpp_net_engine::MakeShared<GameSession>, 
		1);

	ASSERT(pService->Start(), "Connect Failed");


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

	return 0;
}
