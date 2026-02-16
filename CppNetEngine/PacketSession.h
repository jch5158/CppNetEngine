#pragma once
#include "Session.h"

#pragma pack(push, 1)
struct PacketHeader
{
	uint16 size;
	uint32 id;
};
#pragma pack(pop)

class PacketSession : public Session
{
public:
	PacketSession() = default;
	virtual ~PacketSession() override = default;

	PacketSessionRef GetPacketSessionRef();

	virtual int32 OnReceive(byte* pBuffer, const int32 len) override final;
	virtual void OnRecvPacket(byte* buffer, int32 len) = 0;
};

