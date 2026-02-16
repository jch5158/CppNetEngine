#include "pch.h"
#include "PacketSession.h"

PacketSessionRef PacketSession::GetPacketSessionRef()
{
	return static_pointer_cast<PacketSession>(shared_from_this());
}

int32 PacketSession::OnReceive(byte* pBuffer, const int32 len)
{
	int32 processLen = 0;

	while (true)
	{
		const int32 dataSize = len - processLen;
		if (std::cmp_less(dataSize, SIZE_OF_32(PacketHeader)))
		{
			break;
		}

		auto [size, id] = *(reinterpret_cast<PacketHeader*>(&pBuffer[processLen]));

		const int32 sizeWithHeader = size + SIZE_OF_32(PacketHeader);
		if (std::cmp_less(dataSize, sizeWithHeader))
		{
			break;
		}

		OnRecvPacket(&pBuffer[processLen], sizeWithHeader);

		processLen += sizeWithHeader;
	}

	return processLen;
}
