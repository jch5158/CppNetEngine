#include "pch.h"
#include "Generated/ClientLoginPacketHandler.h"
#include "Generated/Login.pb.h"
#include "Generated/PacketId.pb.h"
#include "PacketSession.h"

bool ClientLoginPacketHandler::HANDLE_PACKET_INVALID(PacketSessionRef& session, byte* pBuffer, const uint16 len)
{

	return false;
}

bool ClientLoginPacketHandler::HANDLE_S2C_LOGIN_RES(PacketSessionRef& session, const Protocol::S2C_LOGIN_RES& packet)
{

	return true;
}

bool ClientLoginPacketHandler::HANDLE_S2C_LOGIN_TEST_RES(PacketSessionRef& session,
	const Protocol::S2C_LOGIN_TEST_RES& packet)
{
	return true;
}
