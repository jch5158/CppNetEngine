#pragma once

enum class eIocpEventType : uint8
{
	Accept,
	Connect,
	Receive,
	Send
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

private:

	const eIocpEventType mEventType;
};

class IocpAcceptEvent final : public IocpEvent
{
public:
	IocpAcceptEvent();
};

class IocpConnectEvent final : public IocpEvent
{
public:
	IocpConnectEvent();
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
};