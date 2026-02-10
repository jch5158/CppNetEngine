#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "ObjectAllocator.h"
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

void Listener::Dispatch(class IocpEvent& iocpEvent, int32 numOfBytes)
{
	if (iocpEvent.GetEventType() != eIocpEventType::Accept)
	{
		ASSERT(false, "Listener::Dispatch - eIocpEventType is not Accept");
		return;
	}

	auto* pAcceptEvent = static_cast<IocpAcceptEvent*>(&iocpEvent);
	processAccept(*pAcceptEvent);
}

bool Listener::StartAccept(const NetAddress& netAddress)
{
	if (SocketUtils::CreateTcpSocket(mSocket) == false)
	{
		return false;
	}

	// TODO : IOCP 연동

	if (SocketUtils::SetReuseAddress(mSocket, true) == false)
	{
		return false;
	}

	if (SocketUtils::SetLinger(mSocket, 0, 0) == false)
	{
		return false;
	}

	if (SocketUtils::Bind(mSocket, netAddress) == false)
	{
		return false;
	}

	if (SocketUtils::Listen(mSocket, SOMAXCONN_HINT(65535)) == false)
	{
		return false;
	}
	
	// TODO : AcceptCount
	const auto pAcceptEvent = ObjectAllocator<IocpAcceptEvent>::GetInstance().Alloc();
	registerAccept(*pAcceptEvent);

	return true;
}

void Listener::CloseAccept()
{
	SocketUtils::Close(mSocket);
}

void Listener::registerAccept(IocpAcceptEvent& acceptEvent) const
{
	Session* pSession = ObjectAllocator<Session>::GetInstance().Alloc();
	acceptEvent.Init();
	acceptEvent.SetSession(pSession);

	if (false == SocketUtils::AcceptEx(mSocket, pSession->GetSocket(), pSession->GetReceiveBuffer().GetWritePointer(), static_cast<OVERLAPPED*>(&acceptEvent)))
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
	const auto pSession = acceptEvent.GetClientSession();

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
	registerAccept(acceptEvent);
}
