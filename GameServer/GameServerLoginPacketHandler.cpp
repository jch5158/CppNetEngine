// ReSharper disable CppInconsistentNaming
#include "pch.h"
#include "Generated/GameServerLoginPacketHandler.h"

bool GameServerLoginPacketHandler::HANDLE_PACKET_INVALID(PacketSessionRef& session, byte* pBuffer, const uint16 len)
{
	return true;
}

bool GameServerLoginPacketHandler::HANDLE_C2S_LOGIN_REQ(PacketSessionRef& session,
	const Protocol::C2S_LOGIN_REQ& packet)
{
	return true;
}
