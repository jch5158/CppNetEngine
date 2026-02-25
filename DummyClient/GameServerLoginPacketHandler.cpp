#include "pch.h"
#include "Generated/GameServerLoginPacketHandler.h"

#include "GameSession.h"

bool GameServerLoginPacketHandler::HANDLE_PACKET_INVALID(PacketSessionRef& pSession, byte* pBuffer, const uint16 len)
{
	return true;
}

bool GameServerLoginPacketHandler::HANDLE_S2C_ECHO_RES(PacketSessionRef& pSession, const Protocol::S2C_ECHO_RES& packet)
{
	GameSessionRef pGameSession = std::static_pointer_cast<GameSession>(pSession);

	fmt::print("{}", packet.ehcomsg());

	return true;
}

bool GameServerLoginPacketHandler::HANDLE_S2C_LOGIN_RES(PacketSessionRef& pSession,
	const Protocol::S2C_LOGIN_RES& packet)
{
	return true;
}

bool GameServerLoginPacketHandler::HANDLE_S2C_LOGIN_TEST_RES(PacketSessionRef& pSession,
	const Protocol::S2C_LOGIN_TEST_RES& packet)
{
	return true;
}
