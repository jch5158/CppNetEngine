#include "pch.h"
#include "GameSession.h"

#include "Generated/ClientLoginPacketHandler.h"

GameSession::GameSession()
{
	fmt::print(L"GameSession Created\n");
}

GameSession::~GameSession()
{
	fmt::print(L"GameSession Destroyed\n");
}

void GameSession::OnConnected()
{
	fmt::print(L"Client Connect\n");
}

void GameSession::OnDisconnecting(const eDisconnectReason reason)
{
	fmt::print(L"On Disconnecting : {}\n", static_cast<uint16>(reason));
}

void GameSession::OnDisconnected()
{
	fmt::print(L"Client Disconnect\n");
}

void GameSession::OnSend(const int32 len)
{
}

void GameSession::OnRecvPacket(byte* pBuffer, const int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	auto* header = reinterpret_cast<PacketHeader*>(pBuffer);

	//ClientLoginPacketHandler::GetInstance().HandlePacket(session, pBuffer, static_cast<uint16>(len));
}

void GameSession::OnError(const int32 errorCode)
{
	return;
}
