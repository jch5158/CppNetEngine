#include "pch.h"
#include "IocpEvent.h"

IocpEvent::IocpEvent(const eIocpEventType eventType)
	:mEventType(eventType)
{
}

void IocpEvent::Init()
{
	OVERLAPPED::Internal = 0;
	OVERLAPPED::InternalHigh = 0;
	OVERLAPPED::Offset = 0;
	OVERLAPPED::OffsetHigh = 0;
	OVERLAPPED::hEvent = nullptr;
}

eIocpEventType IocpEvent::GetEventType() const
{
	return mEventType;
}

IocpAcceptEvent::IocpAcceptEvent()
	:IocpEvent(eIocpEventType::Accept)
{
}

IocpConnectEvent::IocpConnectEvent()
	:IocpEvent(eIocpEventType::Connect)
{
}

IocpReceiveEvent::IocpReceiveEvent()
	:IocpEvent(eIocpEventType::Receive)
{
}

IocpSendEvent::IocpSendEvent()
	:IocpEvent(eIocpEventType::Send)
{
}
