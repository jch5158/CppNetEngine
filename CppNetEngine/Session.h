#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "LockFreeQueue.h"
#include "NetAddress.h"
#include "NetReceiveBuffer.h"
#include "SharedPtrUtils.h"

class Session : public IocpObject
{
public:

	static constexpr int32 MAX_SEND_WSABUF_SIZE = 64;
	static constexpr int32 MAX_RECEIVE_WSABUF_SIZE = 2;

	Session(const Session&) = delete;
	Session& operator=(const Session&) = delete;
	Session(Session&&) = delete;
	Session& operator=(Session&&) = delete;

	explicit Session();
	virtual ~Session() override = default;

	[[nodiscard]]
	virtual HANDLE GetHandle() const override;
	virtual void Dispatch(class IocpEvent& iocpEvent, const uint32 numOfBytes) override;

	virtual void OnConnected() = 0;
	virtual void OnDisconnected() = 0;
	virtual void OnSend(const int32 len) = 0;
	virtual int32 OnReceive(byte* pBuffer, const int32 len) = 0;
	virtual void OnError(const int32 errorCode) = 0;

	void SetSessionIndex(const int32 sessionIndex);
	void SetService(const ServiceRef& pService);
	void SetNetAddress(const NetAddress& address);

	int32 GetSessionIndex() const;
	ServiceRef GetService() const;
	SOCKET GetSocket() const;
	NetAddress& GetAddress();
	NetReceiveBuffer<>& GetNetReceiveBuffer();
	SessionRef GetSessionRef();

	bool IsConnected() const;
	bool Connect();
	bool Disconnect();
	void Send(const INetBufferRef& pSendBuffer);

	bool RegisterConnect();
	bool RegisterDisconnect();
	void RegisterSend();
	void RegisterReceive();

	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessSend(const uint32 numOfBytes);
	void ProcessReceive(const uint32 numOfBytes);

private:

	int32 mSessionIndex;
	IocpConnectEvent mConnectEvent;
	IocpDisconnectEvent mDisconnectEvent;
	IocpReceiveEvent mReceiveEvent;
	IocpSendEvent mSendEvent;

	WeakServiceRef mpService;
	SOCKET mSocket;
	NetAddress mNetAddress;
	std::atomic<bool> mbConnected;
	NetReceiveBuffer<> mNetReceiveBuffer;
	std::atomic<bool> mbSendRegistered;
	LockFreeQueue<INetBufferRef> mSendQueue;
};

