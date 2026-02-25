#include "pch.h"
#include "GameSession.h"

#include "Generated/GameServerLoginPacketHandler.h"

void GameSession::OnConnected()
{
	Protocol::C2S_ECHO_REQ packet;
	packet.set_ehcomsg("Hello World\n");
	const auto pSendBuffer = GameServerLoginPacketHandler::MakeSendBuffer(packet);
	Send(pSendBuffer);
}

void GameSession::OnEnterWaitQueue()
{
}

void GameSession::OnDisconnecting(const eDisconnectReason reason)
{
}

void GameSession::OnDisconnected()
{
}

void GameSession::OnSend(const int32 len)
{
}

void GameSession::OnRecvPacket(byte* pBuffer, const int32 len)
{
	PacketSessionRef pSession = GetPacketSessionRef();

	if (GameServerLoginPacketHandler::GetInstance().HandlePacket(pSession, pBuffer, static_cast<uint16>(len)) == false)
	{
		pSession->Disconnect();
	}
}

void GameSession::OnError(const int32 errorCode)
{
}