// ReSharper disable CppInconsistentNaming
#pragma once
#include "PacketId.pb.h"
#include "Enum.pb.h"
#include "Struct.pb.h"
#include "Login.pb.h"
#include "StlTypes.h"
#include "PacketSession.h"
#include <functional>

class GameServerLoginPacketHandler final : public ISingleton<GameServerLoginPacketHandler>
{
public:
	
    friend class ISingleton<GameServerLoginPacketHandler>;

	using PacketHandle = std::function<bool(PacketSessionRef&, byte*, uint16)>;
	
	GameServerLoginPacketHandler()
		:mPacketHandleMap()
	{
		
		mPacketHandleMap[Protocol::ePacketId::ID_S2C_ECHO_RES] = [this](PacketSessionRef& pSession, byte* pBuffer, const uint16 len)->bool
		{
		    return HandlePacket<Protocol::S2C_ECHO_RES>(HANDLE_S2C_ECHO_RES, pSession, pBuffer, len);
		};
		
		mPacketHandleMap[Protocol::ePacketId::ID_S2C_LOGIN_RES] = [this](PacketSessionRef& pSession, byte* pBuffer, const uint16 len)->bool
		{
		    return HandlePacket<Protocol::S2C_LOGIN_RES>(HANDLE_S2C_LOGIN_RES, pSession, pBuffer, len);
		};
		
		mPacketHandleMap[Protocol::ePacketId::ID_S2C_LOGIN_TEST_RES] = [this](PacketSessionRef& pSession, byte* pBuffer, const uint16 len)->bool
		{
		    return HandlePacket<Protocol::S2C_LOGIN_TEST_RES>(HANDLE_S2C_LOGIN_TEST_RES, pSession, pBuffer, len);
		};
		
	}

	virtual ~GameServerLoginPacketHandler() override = default;

	bool HandlePacket(PacketSessionRef& pSession, byte* pBuffer, const uint16 len) const
	{
		const auto [size, id] = *(reinterpret_cast<PacketHeader*>(pBuffer));
		const auto iter = mPacketHandleMap.find(id);
		if (iter != mPacketHandleMap.end())
		{
			return iter->second(pSession, pBuffer, len);
		}

		return HANDLE_PACKET_INVALID(pSession, pBuffer, len);
	}

	static bool HANDLE_PACKET_INVALID(PacketSessionRef& pSession, byte* pBuffer, const uint16 len);
    static bool HANDLE_S2C_ECHO_RES(PacketSessionRef& pSession, const Protocol::S2C_ECHO_RES& packet);
    static bool HANDLE_S2C_LOGIN_RES(PacketSessionRef& pSession, const Protocol::S2C_LOGIN_RES& packet);
    static bool HANDLE_S2C_LOGIN_TEST_RES(PacketSessionRef& pSession, const Protocol::S2C_LOGIN_TEST_RES& packet);
    
    
    static INetBufferRef MakeSendBuffer(Protocol::C2S_ECHO_REQ& packet) { return MakeSendBuffer(packet, static_cast<uint16>(Protocol::ID_C2S_ECHO_REQ)); }
    static INetBufferRef MakeSendBuffer(Protocol::C2S_LOGIN_REQ& packet) { return MakeSendBuffer(packet, static_cast<uint16>(Protocol::ID_C2S_LOGIN_REQ)); }
    

private:

	template<typename PACKET_TYPE, typename HANDLE>
	bool HandlePacket(HANDLE handlePacket, PacketSessionRef& pSession, byte* pBuffer, const uint16 len) const
	{
		PACKET_TYPE packet{};
		if (packet.ParseFromArray(pBuffer + SIZE_OF_16(PacketHeader), len - SIZE_OF_16(PacketHeader)) == false)
		{
			return false;
		}

		return handlePacket(pSession, packet);
	}

    template<typename T>
	static INetBufferRef MakeSendBuffer(T& packet, const uint16 packetId)
	{
		const uint16 dataSize = static_cast<uint16>(packet.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		auto sendBuffer = cpp_net_engine::MakeSendBuffer(packetSize);

		byte* pBuffer = sendBuffer->Reserve(packetSize);
		if (pBuffer == nullptr)
		{
			NET_ENGINE_LOG_FATAL("MakeSendBuffer sendBuffer->Reserve(packetSize) is failed");
			CrashReporter::Crash();
		}

		auto* header = reinterpret_cast<PacketHeader*>(pBuffer);
		header->size = packetSize;
		header->id = packetId;
		if (!packet.SerializeToArray(&header[1], dataSize))
		{
			NET_ENGINE_LOG_FATAL("GameServerLoginPacketHandler::MakeSendBuffer SerializeToArray is Failed, &header[1] : {}, packetId : {}, dataSize : {}", fmt::ptr(&header[1]), header->id, dataSize);
			CrashReporter::Crash();
	    }
		
        sendBuffer->Commit(packetSize);
		
        return sendBuffer;
	}

	HashMap<uint32, PacketHandle> mPacketHandleMap;
};