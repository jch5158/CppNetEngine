#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "UniquePtrUtils.h"

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

	mAcceptEvents.emplace_back(UniquePtrUtils<IocpAcceptEvent>::Alloc());

	return true;
}

bool Listener::CloseAccept()
{
	return false;
}

void Listener::registerAccept(IocpAcceptEvent* pAcceptEvent)
{
}

void Listener::processAccept(IocpAcceptEvent* pAcceptEvent)
{
}
