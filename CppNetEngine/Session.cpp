#include "pch.h"
#include "Session.h"
#include "CrashReporter.h"
#include "Service.h"
#include "SocketUtils.h"
#include "NetSendBuffer.h"

Session::Session()
	: mSessionIndex(-1)
	, mConnectEvent()
	, mDisconnectEvent()
	, mReceiveEvent()
	, mSendEvent()
	, mpService()
	, mSocket(INVALID_SOCKET)
	, mNetAddress()
	, mbConnected(false)
	, mNetReceiveBuffer()
	, mSendQueue(65535)
{
	if (SocketUtils::CreateTcpSocket(mSocket) == false)
	{
		CrashReporter::Crash();
	}
}

HANDLE Session::GetHandle() const
{
	return reinterpret_cast<HANDLE>(mSocket);  // NOLINT(performance-no-int-to-ptr)
}

void Session::Dispatch(IocpEvent& iocpEvent, const uint32 numOfBytes)
{
	switch (iocpEvent.GetEventType())  // NOLINT(clang-diagnostic-switch-enum)
	{
	case eIocpEventType::Connect:
		ProcessConnect();
		break;
	case eIocpEventType::Disconnect:
		ProcessDisconnect();
		break;
	case eIocpEventType::Send:
		ProcessSend(numOfBytes);
		break;
	case eIocpEventType::Receive:
		ProcessReceive(numOfBytes);
		break;
	default:
		NET_ENGINE_LOG_ERROR("Session::Dispatch - iocp event type is unmatched, iocpEvent.GetEventType() : {}", static_cast<uint8>(iocpEvent.GetEventType()));
		break;
	}
}

void Session::SetSessionIndex(const int32 sessionIndex)
{
	mSessionIndex = sessionIndex;
}

void Session::SetService(const ServiceRef& pService)
{
	mpService = pService;
}

void Session::SetNetAddress(const NetAddress& address)
{
	mNetAddress = address;
}

int32 Session::GetSessionIndex() const
{
	return mSessionIndex;
}

ServiceRef Session::GetService() const
{
	return mpService.lock();
}

SOCKET Session::GetSocket() const
{
	return mSocket;
}

NetAddress& Session::GetAddress()
{
	return mNetAddress;
}

NetReceiveBuffer<>& Session::GetNetReceiveBuffer()
{
	return mNetReceiveBuffer;
}

SessionRef Session::GetSessionRef()
{
	return std::static_pointer_cast<Session>(shared_from_this());
}

bool Session::IsConnected() const
{
	return mbConnected.load();
}

bool Session::Connect()
{
	return RegisterConnect();
}

bool Session::Disconnect()
{
	return RegisterDisconnect();
}

void Session::Send(const INetBufferRef& pSendBuffer)
{
	if (!IsConnected())
	{
		return;
	}

	if (mSendQueue.TryEnqueue(pSendBuffer) == false)
	{
		NET_ENGINE_LOG_FATAL("Session::Send - TryEnqueue is Failed, mSendQueue.Count() : {}", mSendQueue.Count());
		return;
	}

	RegisterSend();
}

bool Session::RegisterConnect()
{
	if (IsConnected())
	{
		return false;
	}

	if (GetService()->GetServiceType() != eServiceType::Client)
	{
		return false;
	}

	if (SocketUtils::SetReuseAddress(mSocket, true) == false)
	{
		return false;
	}

	if (SocketUtils::BindAnyAddress(mSocket, 0) == false)
	{
		return false;
	}

	mConnectEvent.Init();
	mConnectEvent.SetIocpObjectRef(shared_from_this());

	if (false == SocketUtils::ConnectEx(mSocket, GetService()->GetNetAddress().GetSockAddr(), &mConnectEvent))
	{
		const int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			OnError(errorCode);
			mConnectEvent.SetIocpObjectRef(nullptr);
			return false;
		}
	}

	return true;
}

bool Session::RegisterDisconnect()
{
	if (mbConnected.exchange(false) == false)
	{
		return false;
	}

	mDisconnectEvent.Init();
	mDisconnectEvent.SetIocpObjectRef(shared_from_this());

	if (SocketUtils::DisconnectEx(mSocket, &mDisconnectEvent) == false)
	{
		const int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			OnError(errorCode);
			mDisconnectEvent.SetIocpObjectRef(nullptr);
			return false;
		}
	}

	return true;
}

void Session::RegisterSend()
{
	if (!IsConnected())
	{
		return;
	}

	if (mbSendRegistered.exchange(true) == true)
	{
		return;
	}

	mSendEvent.Init();
	mSendEvent.SetIocpObjectRef(shared_from_this());

	WSABUF wsabufs[MAX_SEND_WSABUF_SIZE]{};
	int32 sendCount;
	for (sendCount = 0; sendCount < MAX_SEND_WSABUF_SIZE; ++sendCount)
	{
		INetBufferRef pSendBuffer = nullptr;
		if (mSendQueue.TryDequeue(pSendBuffer) == false)
		{
			break;
		}

		mSendEvent.GetSendPendingBuffer().emplace_back(pSendBuffer);
		wsabufs[sendCount].buf = reinterpret_cast<char*>(pSendBuffer->GetReadPtr());
		wsabufs[sendCount].len = pSendBuffer->GetUseSize();
	}

	if (SocketUtils::WsaSend(mSocket, wsabufs, sendCount, &mSendEvent) == false)
	{
		const int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			OnError(errorCode);
			mSendEvent.SetIocpObjectRef(nullptr);
			mSendEvent.GetSendPendingBuffer().clear();
			mbSendRegistered.store(false);
		}
	}
}

void Session::RegisterReceive()
{
	if (IsConnected() == false)
	{
		return;
	}

	mReceiveEvent.Init();
	mReceiveEvent.SetIocpObjectRef(shared_from_this());

	WSABUF wsabufs[MAX_RECEIVE_WSABUF_SIZE]{};

	const int32 linearSize = mNetReceiveBuffer.GetLinearWriteSize();
	const int32 remainSize = mNetReceiveBuffer.GetFreeSize() - linearSize;

	wsabufs[0].buf = reinterpret_cast<char*>(mNetReceiveBuffer.GetWritePtr());
	wsabufs[0].len = linearSize;

	int32 wsabufsLen = 1;
	if (remainSize != 0)
	{
		++wsabufsLen;
		wsabufs[1].buf = reinterpret_cast<char*>(mNetReceiveBuffer.GetBufferPtr());
		wsabufs[1].len = remainSize;
	}

	if (SocketUtils::WsaReceive(mSocket, wsabufs, wsabufsLen, &mReceiveEvent) == false)
	{
		const int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			OnError(errorCode);
			mReceiveEvent.SetIocpObjectRef(nullptr);
		}
	}
}

void Session::ProcessConnect()
{
	mConnectEvent.ReleaseIocpObjectRef();

	mbConnected.store(true);

	if (GetService()->AddSession(GetSessionRef()) == false)
	{
		Disconnect();
	}

	OnConnected();

	RegisterReceive();
}

void Session::ProcessDisconnect()
{
	mDisconnectEvent.ReleaseIocpObjectRef();

	OnDisconnected();
	GetService()->ReleaseSession(GetSessionRef());
}

void Session::ProcessSend(const uint32 numOfBytes)
{
	mSendEvent.ReleaseIocpObjectRef();
	mSendEvent.GetSendPendingBuffer().clear();

	if (numOfBytes == 0)
	{
		Disconnect();
		return;
	}

	OnSend(static_cast<int32>(numOfBytes));

	if (!mSendQueue.IsEmpty())
	{
		RegisterSend();
	}
}

void Session::ProcessReceive(const uint32 numOfBytes)
{
	mReceiveEvent.ReleaseIocpObjectRef();

	if (numOfBytes == 0)
	{
		Disconnect();
		return;
	}

	mNetReceiveBuffer.MoveWritePos(static_cast<int32>(numOfBytes));

	const int32 dataSize = mNetReceiveBuffer.GetUseSize();
	const int32 processLen = OnReceive(mNetReceiveBuffer.GetReadPtr(), static_cast<int32>(numOfBytes));
	if (processLen < 0 || processLen > dataSize)
	{
		Disconnect();
		return;
	}

	mNetReceiveBuffer.MoveReadPos(processLen);

	RegisterReceive();
}
