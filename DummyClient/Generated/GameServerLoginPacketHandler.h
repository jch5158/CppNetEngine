// ReSharper disable CppInconsistentNaming
#pragma once
#include <functional>
#include "PacketId.pb.h"
#include "Enum.pb.h"
#include "Struct.pb.h"
#include "Login.pb.h"
#include "StlTypes.h"
#include "PacketSession.h"

class GameServerLoginPacketHandler final : ISingleton<GameServerLoginPacketHandler>
{
public:
	
	using PacketHandle = std::function<bool(PacketSessionRef&, byte*, uint16)>;
	
	GameServerLoginPacketHandler()
		:mPacketHandleMap()
	{
		
		mPacketHandleMap[Protocol::ePacketId::ID_S2C_LOGIN_RES] = [this](PacketSessionRef& session, byte* pBuffer, const uint16 len)->bool
		{
		    return HandlePacket<Protocol::S2C_LOGIN_RES>(HANDLE_S2C_LOGIN_RES, session, pBuffer, len);
		};
		
		mPacketHandleMap[Protocol::ePacketId::ID_S2C_LOGIN_TEST_RES] = [this](PacketSessionRef& session, byte* pBuffer, const uint16 len)->bool
		{
		    return HandlePacket<Protocol::S2C_LOGIN_TEST_RES>(HANDLE_S2C_LOGIN_TEST_RES, session, pBuffer, len);
		};
		
	}

	virtual ~GameServerLoginPacketHandler() override = default;

	bool HandlePacket(PacketSessionRef& session, byte* pBuffer, const uint16 len) const
	{
		const auto [size, id] = *(reinterpret_cast<PacketHeader*>(pBuffer));
		const auto iter = mPacketHandleMap.find(id);
		if (iter == mPacketHandleMap.end())
		{
			return iter->second(session, pBuffer, len);
		}

		return HANDLE_PACKET_INVALID(session, pBuffer, len);
	}

	static bool HANDLE_PACKET_INVALID(PacketSessionRef& session, byte* pBuffer, const uint16 len);
    static bool HANDLE_S2C_LOGIN_RES(PacketSessionRef& session, const Protocol::S2C_LOGIN_RES& packet);
    static bool HANDLE_S2C_LOGIN_TEST_RES(PacketSessionRef& session, const Protocol::S2C_LOGIN_TEST_RES& packet);
    
    
    static INetBufferRef MakeSendBuffer(Protocol::S2C_LOGIN_RES& packet) { return MakeSendBuffer(packet, static_cast<uint16>(Protocol::ID_S2C_LOGIN_RES)); }
    static INetBufferRef MakeSendBuffer(Protocol::S2C_LOGIN_TEST_RES& packet) { return MakeSendBuffer(packet, static_cast<uint16>(Protocol::ID_S2C_LOGIN_TEST_RES)); }
    

private:

	template<typename PACKET_TYPE, typename HANDLE>
	bool HandlePacket(HANDLE handlePacket, PacketSessionRef& session, byte* pBuffer, const uint16 len) const
	{
		PACKET_TYPE packet{};
		if (packet.ParseFromArray(pBuffer + SIZE_OF_16(PacketHeader), len - SIZE_OF_16(PacketHeader)) == false)
		{
			return false;
		}

		return handlePacket(session, packet);
	}

    template<typename T>
	static INetBufferRef MakeSendBuffer(T& packet, const uint16 packetId)
	{
		const uint16 dataSize = static_cast<uint16>(packet.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		auto sendBuffer = cpp_net_engine::MakeSendBuffer(packetSize);

		byte* pBuffer = sendBuffer->Reserve(packetSize);

		auto* header = reinterpret_cast<PacketHeader*>(pBuffer);
		header->size = packetSize;
		header->id = packetId;
		ASSERT(packet.SerializeToArray(&header[1], dataSize), "LoginGameServerPacketHandler::MakeSendBuffer SerializeToArray is Failed");
		
        sendBuffer->Commit(packetSize);
		
        return sendBuffer;
	}

	HashMap<uint32, PacketHandle> mPacketHandleMap;
};