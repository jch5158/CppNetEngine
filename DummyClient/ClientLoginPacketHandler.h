// ReSharper disable CppInconsistentNaming
#pragma once
#include <functional>
#include "PacketSession.h"
#include "PacketId.pb.h"
#include "Login.pb.h"

class ClientLoginPacketHandler final : ISingleton<ClientLoginPacketHandler>
{
public:
	
	using PacketHandle = std::function<bool(PacketSessionRef&, byte*, uint16)>;
	
	ClientLoginPacketHandler()
		:mPacketHandles(UINT16_MAX, nullptr)
	{
		for (auto& mPacketHandle : mPacketHandles)
		{
			mPacketHandle = HANDLE_PACKET_INVALID;
		}

		
		mPacketHandles[Protocol::ePacketId::ID_S2C_LOGIN_RES] = [this](PacketSessionRef& session, byte* pBuffer, const uint16 len)->bool
		{
		    return HandlePacket<Protocol::S2C_LOGIN_RES>(HANDLE_S2C_LOGIN_RES, session, pBuffer, len);
		};
		
		mPacketHandles[Protocol::ePacketId::ID_S2C_LOGIN_TEST_RES] = [this](PacketSessionRef& session, byte* pBuffer, const uint16 len)->bool
		{
		    return HandlePacket<Protocol::S2C_LOGIN_TEST_RES>(HANDLE_S2C_LOGIN_TEST_RES, session, pBuffer, len);
		};
		
	}

	virtual ~ClientLoginPacketHandler() override = default;

	bool HandlePacket(PacketSessionRef& session, byte* pBuffer, const uint16 len) const
	{
		const auto [size, id] = *(reinterpret_cast<PacketHeader*>(pBuffer));
		return mPacketHandles[id](session, pBuffer, len);
	}

	static bool HANDLE_PACKET_INVALID(PacketSessionRef& session, byte* pBuffer, const uint16 len);
    
    static bool HANDLE_S2C_LOGIN_RES(PacketSessionRef& session, const Protocol::S2C_LOGIN_RES& packet);
    static bool HANDLE_S2C_LOGIN_TEST_RES(PacketSessionRef& session, const Protocol::S2C_LOGIN_TEST_RES& packet);

private:

	template<typename PACKET_TYPE, typename HANDLE>
	bool HandlePacket(HANDLE handlePacket, PacketSessionRef& session, byte* pBuffer, const uint16 len) const
	{
		PACKET_TYPE packet;
		if (packet.ParseFromArray(pBuffer + SIZE_OF_16(PacketHeader), len - SIZE_OF_16(PacketHeader)) == false)
		{
			return false;
		}

		return handlePacket(session, packet);
	}

	Vector<PacketHandle> mPacketHandles;
};