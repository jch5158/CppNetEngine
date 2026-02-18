using System;
using System.Collections.Generic;
using System.Text;

namespace PacketGenerator
{
    internal static class PacketFormatter
    {
        public static readonly string HANDLE_FILE_FORMAT =
            @"// ReSharper disable CppInconsistentNaming
#pragma once
#include <functional>
#include ""PacketSession.h""
#include ""PacketId.pb.h""
#include ""{0}.pb.h""

class {1}{0}PacketHandler final : ISingleton<{1}{0}PacketHandler>
{{
public:
	
	using PacketHandle = std::function<bool(PacketSessionRef&, byte*, uint16)>;
	
	{1}{0}PacketHandler()
		:mPacketHandles(UINT16_MAX, nullptr)
	{{
		for (auto& mPacketHandle : mPacketHandles)
		{{
			mPacketHandle = HANDLE_PACKET_INVALID;
		}}

		{2}
	}}

	virtual ~{1}{0}PacketHandler() override = default;

	bool HandlePacket(PacketSessionRef& session, byte* pBuffer, const uint16 len) const
	{{
		const auto [size, id] = *(reinterpret_cast<PacketHeader*>(pBuffer));
		return mPacketHandles[id](session, pBuffer, len);
	}}

	static bool HANDLE_PACKET_INVALID(PacketSessionRef& session, byte* pBuffer, const uint16 len);
    {3}

private:

	template<typename PACKET_TYPE, typename HANDLE>
	bool HandlePacket(HANDLE handlePacket, PacketSessionRef& session, byte* pBuffer, const uint16 len) const
	{{
		PACKET_TYPE packet;
		if (packet.ParseFromArray(pBuffer + SIZE_OF_16(PacketHeader), len - SIZE_OF_16(PacketHeader)) == false)
		{{
			return false;
		}}

		return handlePacket(session, packet);
	}}

	Vector<PacketHandle> mPacketHandles;
}};";

        public static readonly string INIT_FILE_FORMAT =
@"
		mPacketHandles[Protocol::ePacketId::{0}] = [this](PacketSessionRef& session, byte* pBuffer, const uint16 len)->bool
		{{
		    return HandlePacket<Protocol::{1}>(HANDLE_{1}, session, pBuffer, len);
		}};
		";


		public static readonly string DECLARE_FILE_FORMAT =
@"
    static bool HANDLE_{0}(PacketSessionRef& session, const Protocol::{0}& packet);";
    }
}
