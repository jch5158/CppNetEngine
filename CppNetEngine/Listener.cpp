#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "ObjectAllocator.h"
#include "Service.h"
#include "Session.h"

Listener::Listener(const int32 acceptCount, std::function<void(const uint32)> pErrorHandle)
	: mSocket(INVALID_SOCKET)
	, mAcceptCount(acceptCount)
	, mpErrorHandle(std::move(pErrorHandle))
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
		NET_ASSERT(false, "Listener::Dispatch - eIocpEventType is not Accept");
		return;
	}

	auto* pAcceptEvent = static_cast<IocpAcceptEvent*>(&iocpEvent);
	processAccept(*pAcceptEvent);
}

bool Listener::StartAccept(const ServerServiceRef& pServerService)
{
	if (pServerService == nullptr)
	{
		NET_ENGINE_LOG_FATAL("pServerService is nullptr");
		CrashReporter::Crash();
	}

	mpServerService = pServerService;

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

	if (SocketUtils::SetKeepAlive(mSocket, 30000, 1000) == false)
	{
		NET_ENGINE_LOG_FATAL("SocketUtils::SetKeepAlive - errorCode : {}", WSAGetLastError());
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
	
	for (int32 i = 0; i < mAcceptCount; ++i)
	{
		auto pAcceptEvent = cpp_net_engine::NewObject<IocpAcceptEvent>();
		pAcceptEvent->ClearOverlapped();
		pAcceptEvent->SetOwner(shared_from_this());
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
	if (pSession == nullptr)
	{
		NET_ENGINE_LOG_FATAL("Listener::registerAccept - mpServerService->CreateSession() is failed");
		return;
	}

	acceptEvent.SetSession(pSession);
	acceptEvent.ClearOverlapped();
	if (false == SocketUtils::AcceptEx(mSocket, pSession->GetSocket(), pSession->GetReceiveBufferPtr(), &acceptEvent))
	{
		const int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			pSession->Clear();
			mpErrorHandle(errorCode);
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

	pSession->setNetAddress(NetAddress(sockAddr));
	pSession->processConnect();
	registerAccept(acceptEvent);
}
