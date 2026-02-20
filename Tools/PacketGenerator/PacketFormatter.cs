namespace PacketGenerator
{
    internal static class PacketFormatter
    {
        public static readonly string HANDLE_FILE_FORMAT =
            @"// ReSharper disable CppInconsistentNaming
#pragma once
#include <functional>
#include ""PacketId.pb.h""
#include ""Enum.pb.h""
#include ""Struct.pb.h""
#include ""{1}.pb.h""
#include ""StlTypes.h""
#include ""PacketSession.h""

class {0}{1}PacketHandler final : ISingleton<{0}{1}PacketHandler>
{{
public:
	
	using PacketHandle = std::function<bool(PacketSessionRef&, byte*, uint16)>;
	
	{0}{1}PacketHandler()
		:mPacketHandleMap()
	{{
		{2}
	}}

	virtual ~{0}{1}PacketHandler() override = default;

	bool HandlePacket(PacketSessionRef& session, byte* pBuffer, const uint16 len) const
	{{
		const auto [size, id] = *(reinterpret_cast<PacketHeader*>(pBuffer));
		const auto iter = mPacketHandleMap.find(id);
		if (iter == mPacketHandleMap.end())
		{{
			return iter->second(session, pBuffer, len);
		}}

		return HANDLE_PACKET_INVALID(session, pBuffer, len);
	}}

	static bool HANDLE_PACKET_INVALID(PacketSessionRef& session, byte* pBuffer, const uint16 len);
    {3}
    
    {4}

private:

	template<typename PACKET_TYPE, typename HANDLE>
	bool HandlePacket(HANDLE handlePacket, PacketSessionRef& session, byte* pBuffer, const uint16 len) const
	{{
		PACKET_TYPE packet{{}};
		if (packet.ParseFromArray(pBuffer + SIZE_OF_16(PacketHeader), len - SIZE_OF_16(PacketHeader)) == false)
		{{
			return false;
		}}

		return handlePacket(session, packet);
	}}

    template<typename T>
	static INetBufferRef MakeSendBuffer(T& packet, const uint16 packetId)
	{{
		const uint16 dataSize = static_cast<uint16>(packet.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		auto sendBuffer = cpp_net_engine::MakeSendBuffer(packetSize);

		byte* pBuffer = sendBuffer->Reserve(packetSize);

		auto* header = reinterpret_cast<PacketHeader*>(pBuffer);
		header->size = packetSize;
		header->id = packetId;
		ASSERT(packet.SerializeToArray(&header[1], dataSize), ""{1}{0}PacketHandler::MakeSendBuffer SerializeToArray is Failed"");
		
        sendBuffer->Commit(packetSize);
		
        return sendBuffer;
	}}

	HashMap<uint32, PacketHandle> mPacketHandleMap;
}};";

        public static readonly string INIT_FILE_FORMAT =
@"
		mPacketHandleMap[Protocol::ePacketId::{0}] = [this](PacketSessionRef& session, byte* pBuffer, const uint16 len)->bool
		{{
		    return HandlePacket<Protocol::{1}>(HANDLE_{1}, session, pBuffer, len);
		}};
		";


		public static readonly string DECLARE_FILE_FORMAT =
            @"static bool HANDLE_{0}(PacketSessionRef& session, const Protocol::{0}& packet);
    ";

        public static readonly string MAKE_SEND_BUFFER_FUNCTION_FORMAT =
            @"static INetBufferRef MakeSendBuffer(Protocol::{0}& packet) {{ return MakeSendBuffer(packet, static_cast<uint16>(Protocol::{1})); }}
    ";
    }
}
