#include "pch.h"
#include "GameSession.h"

void GameSession::OnConnected()
{
	fmt::print(L"Connected\n");
}

void GameSession::OnDisconnected()
{
}

void GameSession::OnSend(const int32 len)
{
	
}

void GameSession::OnRecvPacket(byte* pBuffer, const int32 len)
{

}

void GameSession::OnError(const int32 errorCode)
{
	
}