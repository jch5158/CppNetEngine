#include "pch.h"
#include "Generated/GameServerLoginPacketHandler.h"

bool GameServerLoginPacketHandler::HANDLE_PACKET_INVALID(PacketSessionRef& session, byte* pBuffer, const uint16 len)
{
	return true;
}

bool GameServerLoginPacketHandler::HANDLE_S2C_LOGIN_RES(PacketSessionRef& session,
	const Protocol::S2C_LOGIN_RES& packet)
{
	return true;
}

bool GameServerLoginPacketHandler::HANDLE_S2C_LOGIN_TEST_RES(PacketSessionRef& session,
	const Protocol::S2C_LOGIN_TEST_RES& packet)
{
	return true;
}
