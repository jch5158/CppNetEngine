#include "pch.h"


#include "GameSession.h"
#include "IocpCore.h"
#include "Service.h"
#include "JobScheduler.h"

int main()
{
	NetEngineInit netEngineInit;

	ServerServiceRef service = cpp_net_engine::MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		cpp_net_engine::MakeShared<IocpCore>(),
		cpp_net_engine::MakeShared<JobScheduler>(),
		cpp_net_engine::MakeShared<GameSession>,
		100);

	service->Start();

	NET_ENGINE_LOG_INFO("GameServer is started.");
}

