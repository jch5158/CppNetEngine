#include "pch.h"

#include "NetAddress.h"

#include "StlAllocator.h"

NetAddress::NetAddress(const SOCKADDR_IN sockAddr)
	: mSockAddr(sockAddr)
{
}

NetAddress::NetAddress(const Wstring& ip, const uint16 port)
	: mSockAddr{}
{
	std::memset(&mSockAddr, 0, sizeof(mSockAddr));
	mSockAddr.sin_family = AF_INET;
	mSockAddr.sin_addr = IpToAddress(ip);
	mSockAddr.sin_port = ::htons(port);
}

const SOCKADDR_IN& NetAddress::GetSockAddr() const
{
	return mSockAddr;
}

Wstring NetAddress::GetIpAddress() const
{
	WCHAR buffer[100];
	::InetNtopW(AF_INET, &mSockAddr.sin_addr, buffer, ARRAY_LEN_16(buffer));
	return { buffer };
}

uint16 NetAddress::GetPort() const
{
	return ::ntohs(mSockAddr.sin_port);
}

IN_ADDR NetAddress::IpToAddress(const Wstring& ip)
{
	IN_ADDR address;
	::InetPtonW(AF_INET, ip.c_str(), &address);
	return address;
}
