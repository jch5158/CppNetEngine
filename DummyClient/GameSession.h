#pragma once
#include "PacketSession.h"

class GameSession final : public PacketSession
{
public:
	virtual void OnConnected() override;
	virtual void OnEnterWaitQueue(const int32 waitTicket) override;
	virtual void OnDisconnecting(const eDisconnectReason reason) override;
	virtual void OnDisconnected() override;
	virtual void OnSend(const int32 len) override;
	virtual void OnRecvPacket(byte* pBuffer, const int32 len) override;
	virtual void OnError(const int32 errorCode) override;

private:
};
