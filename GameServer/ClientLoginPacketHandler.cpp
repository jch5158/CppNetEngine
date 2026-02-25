#include "pch.h"
#include "Generated/ClientLoginPacketHandler.h"
#include "GameSession.h"

bool ClientLoginPacketHandler::HANDLE_PACKET_INVALID(PacketSessionRef& pSession, byte* pBuffer, const uint16 len)
{
	return true;
}

bool ClientLoginPacketHandler::HANDLE_C2S_ECHO_REQ(PacketSessionRef& pSession, const Protocol::C2S_ECHO_REQ& packet)
{
	GameSessionRef pGameSession = std::static_pointer_cast<GameSession>(pSession);

	fmt::print("{}\n", packet.ehcomsg());

	Protocol::S2C_ECHO_RES retPacket;
	retPacket.set_ehcomsg("Hello Client\n");
	const auto pSendBuffer = MakeSendBuffer(retPacket);

	pGameSession->Send(pSendBuffer);

	return true;
}

bool ClientLoginPacketHandler::HANDLE_C2S_LOGIN_REQ(PacketSessionRef& pSession, const Protocol::C2S_LOGIN_REQ& packet)
{
	return true;
}
