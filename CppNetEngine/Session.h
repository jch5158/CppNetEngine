#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "LockFreeQueue.h"
#include "NetAddress.h"
#include "ReceiveBuffer.h"
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
	virtual void Dispatch(class IocpEvent& iocpEvent, const int32 numOfBytes) override;

	virtual void OnConnected() {}
	virtual void OnDisconnected() {}
	virtual void OnSend(const int32 len) {}
	virtual int32 OnReceive(byte* buffer, const int32 len) { return len; }
	virtual void OnError(const int32 errorCode) {}

	void SetSessionIndex(const int32 sessionIndex);
	void SetService(const ServiceRef& pService);
	void SetNetAddress(const NetAddress& address);

	int32 GetSessionIndex() const;
	ServiceRef GetService() const;
	SOCKET GetSocket() const;
	NetAddress& GetAddress();
	ReceiveBuffer& GetReceiveBuffer();
	SessionRef GetSessionRef();

	bool IsConnected() const;
	bool Connect();
	bool Disconnect();
	void Send(const SendBufferRef& pSendBuffer);

	bool RegisterConnect();
	bool RegisterDisconnect();
	void RegisterSend();
	void RegisterReceive();

	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessSend(const int32 numOfBytes);
	void ProcessReceive(const int32 numOfBytes);

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
	ReceiveBuffer mReceiveBuffer;
	std::atomic<bool> mbSendRegistered;
	LockFreeQueue<SendBufferRef> mSendQueue;
};

