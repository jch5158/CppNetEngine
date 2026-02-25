#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "ObjectAllocator.h"
#include "Service.h"
#include "Session.h"

Listener::Listener()
	: mSocket(INVALID_SOCKET)
	, mAcceptEvents()
{
}

Listener::~Listener()
{
	SocketUtils::Close(mSocket);
}

HANDLE Listener::GetHandle() const
{
	return reinterpret_cast<HANDLE>(mSocket);  // NOLINT(performance-no-int-to-ptr)
}

void Listener::Dispatch(class IocpEvent& iocpEvent, uint32 numOfBytes)
{
	if (iocpEvent.GetEventType() != eIocpEventType::Accept)
	{
		ASSERT(false, "Listener::Dispatch - eIocpEventType is not Accept");
		return;
	}

	auto* pAcceptEvent = static_cast<IocpAcceptEvent*>(&iocpEvent);
	processAccept(*pAcceptEvent);
}

bool Listener::StartAccept(ServerServiceRef pServerService)
{
	if (pServerService == nullptr)
	{
		NET_ENGINE_LOG_FATAL("pServerService is nullptr");
		CrashReporter::Crash();
	}

	mpServerService = std::move(pServerService);

	if (SocketUtils::CreateTcpSocket(mSocket) == false)
	{
		NET_ENGINE_LOG_FATAL("SocketUtils::CreateTcpSocket is failed - errorCode : {}", WSAGetLastError());
		CrashReporter::Crash();
	}

	if (mpServerService->GetIocpCore()->Register(shared_from_this()) == false)
	{
		NET_ENGINE_LOG_FATAL("mpServerService->GetIocpCore()->Register is failed - errorCode : {}", WSAGetLastError());
		CrashReporter::Crash();
	}

	if (SocketUtils::SetReuseAddress(mSocket, true) == false)
	{
		NET_ENGINE_LOG_FATAL("SocketUtils::SetReuseAddress is failed - errorCode : {}", WSAGetLastError());
		CrashReporter::Crash();
	}

	if (SocketUtils::SetLinger(mSocket, 1, 0) == false)
	{
		NET_ENGINE_LOG_FATAL("SocketUtils::SetLinger is failed - errorCode : {}", WSAGetLastError());
		CrashReporter::Crash();
	}

	if (SocketUtils::Bind(mSocket, mpServerService->GetNetAddress().GetSockAddr()) == false)
	{
		NET_ENGINE_LOG_FATAL("SocketUtils::Bind is failed - errorCode : {}", WSAGetLastError());
		CrashReporter::Crash();
	}

	if (SocketUtils::Listen(mSocket, SOMAXCONN_HINT(65535)) == false)
	{
		NET_ENGINE_LOG_FATAL("SocketUtils::Listen is failed - errorCode : {}", WSAGetLastError());
		CrashReporter::Crash();
	}
	
	const int32 acceptEventCount = mpServerService->GetMaxSessionCount();
	for (int32 i = 0; i < acceptEventCount; ++i)
	{
		auto pAcceptEvent = cpp_net_engine::NewObject<IocpAcceptEvent>();
		pAcceptEvent->Init();
		pAcceptEvent->SetIocpObjectRef(shared_from_this());
		mAcceptEvents.emplace_back(pAcceptEvent);
		registerAccept(*pAcceptEvent);
	}

	return true;
}

void Listener::CloseAccept()
{
	SocketUtils::Close(mSocket);
}

void Listener::registerAccept(IocpAcceptEvent& acceptEvent) const
{
	const SessionRef pSession = mpServerService->CreateSession();
	acceptEvent.Init();
	acceptEvent.SetSession(pSession);

	if (false == SocketUtils::AcceptEx(mSocket, pSession->GetSocket(), pSession->GetNetReceiveBuffer().GetWritePtr(),&acceptEvent))
	{
		const int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			registerAccept(acceptEvent);
		}
	}
}

void Listener::processAccept(IocpAcceptEvent& acceptEvent) const
{
	const SessionRef pSession = acceptEvent.GetClientSession();

	if (SocketUtils::SetUpdateAcceptSocket(pSession->GetSocket(), mSocket) == false)
	{
		registerAccept(acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddr{};
	int32 sizeOfSockAddr = SIZE_OF_32(sockAddr);
	if (SOCKET_ERROR == getpeername(pSession->GetSocket(), reinterpret_cast<SOCKADDR*>(&sockAddr), &sizeOfSockAddr))
	{
		registerAccept(acceptEvent);
		return;
	}

	pSession->SetNetAddress(NetAddress(sockAddr));
	pSession->ProcessConnect();
	registerAccept(acceptEvent);
}
