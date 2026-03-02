#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "LockFreeQueue.h"
#include "NetAddress.h"
#include "NetReceiveBuffer.h"
#include "SharedPtrUtils.h"

enum class eSessionState : uint8
{
	Connected,
	Waiting,    // 대기열 상태
	InGame,
	Disconnected
};

// 1. 끊김 사유를 명확히 정의
enum class eDisconnectReason : uint16  // NOLINT(performance-enum-size)
{
	ClientRequest,  // 클라이언트가 정상 종료
	Timeout,        // 하트비트 응답 없음
	Kicked,         // 서버에서 강퇴
	ServerFull,		// 대기큐마저 꽉 찼을 때
	ServerShutdown, // 서버 종료
	SocketError     // 네트워크 에러
};

class Session : public IocpObject
{
public:

	friend class Listener;
	friend class Service;
	friend class SessionManager;
	friend class SessionReaper;
	
	static constexpr int32 MAX_SEND_WSABUF_SIZE = 64;
	static constexpr int32 MAX_RECEIVE_WSABUF_SIZE = 2;

	Session(const Session&) = delete;
	Session& operator=(const Session&) = delete;
	Session(Session&&) = delete;
	Session& operator=(Session&&) = delete;

	explicit Session();
	virtual ~Session() override = default;

	[[nodiscard]] virtual HANDLE GetHandle() const override;
	virtual void Dispatch(class IocpEvent& iocpEvent, const uint32 numOfBytes) override;

	virtual void OnConnected() = 0;
	virtual void OnEnterWaitQueue(const int32 waitTicket) = 0;
	virtual void OnDisconnecting(const eDisconnectReason reason) = 0;
	virtual void OnDisconnected() = 0;
	virtual void OnSend(const int32 len) = 0;
	virtual int32 OnReceive(byte* pBuffer, const int32 len) = 0;
	virtual void OnError(const int32 errorCode) = 0;
	virtual void OnActivityUpdate() = 0;
	virtual int64 OnGetLastActivityMs() = 0;

	[[nodiscard]] bool SetSessionInGame();

	[[nodiscard]] ServiceRef GetService() const;
	[[nodiscard]] SOCKET GetSocket() const;
	[[nodiscard]] NetAddress& GetAddress();
	[[nodiscard]] NetReceiveBuffer& GetNetReceiveBuffer();
	[[nodiscard]] SessionRef GetSessionRef();
	
	[[nodiscard]] bool IsInGame() const;
	[[nodiscard]] bool IsConnected() const;
	[[nodiscard]] bool IsDisconnected() const;
	[[nodiscard]] bool Connect();
	bool Disconnect(const eDisconnectReason reason);
	void Send(const NetSendBufferRef& pSendBuffer);

private:

	bool registerConnect();
	bool registerDisconnect();
	void registerSend();
	void registerReceive();
	void registerReapSelf();

	void processConnect();
	void processDisconnect();
	void processSend(const uint32 numOfBytes);
	void processReceive(const uint32 numOfBytes);

	void setService(const ServiceRef& pService);
	void setNetAddress(const NetAddress& address);
	bool setSessionWaiting();
	bool setWaitingToConnected();
	bool setSessionConnected();
	bool setSessionDisconnected();

	IocpConnectEvent mConnectEvent;
	IocpDisconnectEvent mDisconnectEvent;
	IocpReceiveEvent mReceiveEvent;
	IocpSendEvent mSendEvent;

	ServiceWeak mpService;
	SOCKET mSocket;
	NetAddress mNetAddress;
	std::atomic<eSessionState> mSessionState;
	NetReceiveBuffer mNetReceiveBuffer;
	std::atomic<bool> mbSendRegistered;
	LockFreeQueue<NetSendBufferRef> mSendQueue;
};

