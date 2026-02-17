#pragma once
#include "LockFreeQueue.h"
#include "SharedPtrUtils.h"

class Session;

enum class eIocpEventType : uint8
{
	Accept,
	Connect,
	Disconnect,
	Send,
	Receive
};

class IocpEvent : public OVERLAPPED
{
public:

	IocpEvent(const IocpEvent&) = delete;
	IocpEvent& operator=(const IocpEvent&) = delete;
	IocpEvent(IocpEvent&&) = delete;
	IocpEvent& operator=(IocpEvent&&) = delete;

	explicit IocpEvent(const eIocpEventType eventType);
	~IocpEvent() = default;

	void Init();

	[[nodiscard]]
	eIocpEventType GetEventType() const;

	[[nodiscard]]
	IocpObjectRef GetIocpObjectRef();

	void SetIocpObjectRef(const IocpObjectRef& iocpObjectRef);

	void ReleaseIocpObjectRef();

private:

	const eIocpEventType mEventType;
	IocpObjectRef mIocpObjectRef;
};

class IocpAcceptEvent final : public IocpEvent
{
public:
	IocpAcceptEvent();

	void SetSession(SessionRef pClientSession);

	[[nodiscard]]
	SessionRef GetClientSession() const;

private:
	SessionRef mpClientSession;
};

class IocpConnectEvent final : public IocpEvent
{
public:
	IocpConnectEvent();
};

class IocpDisconnectEvent final : public IocpEvent
{
public:
	IocpDisconnectEvent();
};

class IocpReceiveEvent final : public IocpEvent
{
public:
	IocpReceiveEvent();
};

class IocpSendEvent final : public IocpEvent
{
public:
	IocpSendEvent();

	[[nodiscard]]
	Vector<INetBufferRef>& GetSendPendingBuffer();

private:
	Vector<INetBufferRef> mSendPendingBuffer; // 전송중인 버퍼
};