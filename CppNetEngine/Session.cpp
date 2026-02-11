#include "pch.h"
#include "Session.h"
#include "CrashReporter.h"
#include "SocketUtils.h"

Session::Session()
	: mSocket(INVALID_SOCKET)
	, mNetAddress()
	, mbConnected(false)
	, mReceiveBuffer()
	, mSendQueue(65535)
	, mSendPendingQueue(65535)
{
	if (SocketUtils::CreateTcpSocket(mSocket) == false)
	{
		CrashReporter::Crash();
	}
}

Session::~Session()
{
	SocketUtils::Close(mSocket);
}

HANDLE Session::GetHandle() const
{
	return reinterpret_cast<HANDLE>(mSocket);  // NOLINT(performance-no-int-to-ptr)
}

void Session::Dispatch(IocpEvent& iocpEvent, int32 numOfBytes)
{
}

void Session::SetNetAddress(const NetAddress& address)
{
	mNetAddress = address;
}

SOCKET Session::GetSocket() const
{
	return mSocket;
}

NetAddress& Session::GetAddress()
{
	return mNetAddress;
}

ReceiveBuffer& Session::GetReceiveBuffer()
{
	return mReceiveBuffer;
}
