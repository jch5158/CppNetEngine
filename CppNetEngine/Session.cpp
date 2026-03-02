#include "pch.h"
#include "Session.h"
#include "CrashReporter.h"
#include "Service.h"
#include "SocketUtils.h"
#include "NetSendBuffer.h"

Session::Session()
	: mConnectEvent()
	, mDisconnectEvent()
	, mReceiveEvent()
	, mSendEvent()
	, mpService()
	, mSocket(INVALID_SOCKET)
	, mNetAddress()
	, mSessionState(eSessionState::Disconnected)
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
		processConnect();
		break;
	case eIocpEventType::Disconnect:
		processDisconnect();
		break;
	case eIocpEventType::Send:
		processSend(numOfBytes);
		break;
	case eIocpEventType::Receive:
		processReceive(numOfBytes);
		break;
	default:
		NET_ENGINE_LOG_ERROR("Session::Dispatch - iocp event type is unmatched, iocpEvent.GetEventType() : {}", static_cast<uint8>(iocpEvent.GetEventType()));
		break;
	}
}

bool Session::SetSessionInGame()
{
	auto expected = eSessionState::Connected;
	return mSessionState.compare_exchange_weak(expected, eSessionState::InGame);
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

NetReceiveBuffer& Session::GetNetReceiveBuffer()
{
	return mNetReceiveBuffer;
}

SessionRef Session::GetSessionRef()
{
	return std::static_pointer_cast<Session>(shared_from_this());
}

bool Session::IsInGame() const
{
	return mSessionState.load() == eSessionState::InGame;
}

bool Session::IsConnected() const
{
	return mSessionState.load() == eSessionState::Connected;
}

bool Session::IsDisconnected() const
{
	return mSessionState.load() == eSessionState::Disconnected;
}

bool Session::Connect()
{
	return registerConnect();
}

bool Session::Disconnect(const eDisconnectReason reason)
{
	OnDisconnecting(reason);
	return registerDisconnect();
}

void Session::Send(const NetSendBufferRef& pSendBuffer)
{
	if (IsDisconnected())
	{
		return;
	}

	if (mSendQueue.TryEnqueue(pSendBuffer) == false)
	{
		NET_ENGINE_LOG_FATAL("Session::Send - TryEnqueue is Failed, mSendQueue.Count() : {}", mSendQueue.Count());
		return;
	}

	registerSend();
}

bool Session::registerConnect()
{
	if (!IsDisconnected())
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
			Disconnect(eDisconnectReason::SocketError);
			mConnectEvent.SetIocpObjectRef(nullptr);
			return false;
		}
	}

	return true;
}

bool Session::registerDisconnect()
{
	if (!setSessionDisconnected())
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

void Session::registerSend()
{
	if (IsDisconnected())
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
		NetSendBufferRef pSendBuffer;
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
			Disconnect(eDisconnectReason::SocketError);
			mSendEvent.SetIocpObjectRef(nullptr);
			mSendEvent.GetSendPendingBuffer().clear();
			mbSendRegistered.store(false);
		}
	}
}

void Session::registerReceive()
{
	if (IsDisconnected())
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
			Disconnect(eDisconnectReason::SocketError);
			mReceiveEvent.SetIocpObjectRef(nullptr);
		}
	}
}

void Session::registerReap()
{
	GetService()->RegisterSessionReap(GetSessionRef());
}

void Session::processConnect()
{
	mConnectEvent.ReleaseIocpObjectRef();

	if (GetService()->AddSession(GetSessionRef()) == true)
	{
		if (setSessionConnected())
		{
			OnConnected();
		}
	}
	else
	{
		const int32 waitTicket = GetService()->EnterWaitQueue(GetSessionRef());
		if (waitTicket == -1)
		{
			Disconnect(eDisconnectReason::ServerFull);
			return;
		}

		if (setSessionWaiting())
		{
			OnEnterWaitQueue(waitTicket);
		}
	}

	registerReap();
	registerReceive();
}

void Session::processDisconnect()
{
	mDisconnectEvent.ReleaseIocpObjectRef();

	GetService()->ReleaseSession(GetSessionRef());

	const SessionRef pWaitSession = GetService()->DequeueWaitQueue();
	if (pWaitSession != nullptr)
	{
		if (pWaitSession->setWaitingToConnected())
		{
			pWaitSession->OnConnected();
		}
	}
}

void Session::processSend(const uint32 numOfBytes)
{
	mSendEvent.ReleaseIocpObjectRef();
	mSendEvent.GetSendPendingBuffer().clear();

	if (numOfBytes == 0)
	{
		Disconnect(eDisconnectReason::Kicked);
		return;
	}

	OnSend(static_cast<int32>(numOfBytes));

	mbSendRegistered.store(false);

	if (!mSendQueue.IsEmpty())
	{
		registerSend();
	}
}

void Session::processReceive(const uint32 numOfBytes)
{
	mReceiveEvent.ReleaseIocpObjectRef();

	if (numOfBytes == 0)
	{
		Disconnect(eDisconnectReason::Kicked);
		return;
	}

	mNetReceiveBuffer.MoveWritePos(static_cast<int32>(numOfBytes));

	const int32 dataSize = mNetReceiveBuffer.GetUseSize();
	const int32 processLen = OnReceive(mNetReceiveBuffer.GetReadPtr(), static_cast<int32>(numOfBytes));
	if (processLen < 0 || processLen > dataSize)
	{
		Disconnect(eDisconnectReason::Kicked);
		return;
	}

	mNetReceiveBuffer.MoveReadPos(processLen);

	registerReceive();
}

void Session::setService(const ServiceRef& pService)
{
	mpService = pService;
}

void Session::setNetAddress(const NetAddress& address)
{
	mNetAddress = address;
}

bool Session::setSessionWaiting()
{
	auto expected = eSessionState::Disconnected;
	return mSessionState.compare_exchange_weak(expected, eSessionState::Disconnected);
}

bool Session::setWaitingToConnected()
{
	auto expected = eSessionState::Waiting;
	return mSessionState.compare_exchange_weak(expected, eSessionState::Connected);
}

bool Session::setSessionConnected()
{
	auto expected = eSessionState::Disconnected;
	return mSessionState.compare_exchange_weak(expected, eSessionState::Connected);
}

bool Session::setSessionDisconnected()
{
	return mSessionState.exchange(eSessionState::Disconnected) != eSessionState::Disconnected;
}