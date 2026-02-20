#include "pch.h"
#include "Generated/ClientLoginPacketHandler.h"

bool ClientLoginPacketHandler::HANDLE_PACKET_INVALID(PacketSessionRef& session, byte* pBuffer, const uint16 len)
{
	return true;
}

bool ClientLoginPacketHandler::HANDLE_C2S_LOGIN_REQ(PacketSessionRef& session, const Protocol::C2S_LOGIN_REQ& packet)
{
	return true;
}
