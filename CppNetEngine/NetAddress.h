#pragma once

class NetAddress
{
public:

	explicit NetAddress() = default;
	explicit NetAddress(const SOCKADDR_IN sockAddr);
	explicit NetAddress(const Wstring& ip, const uint16 port);

	[[nodiscard]]
	const SOCKADDR_IN& GetSockAddr() const;
	
	[[nodiscard]]
	Wstring	GetIpAddress() const;

	[[nodiscard]]
	uint16 GetPort() const;

	static IN_ADDR	IpToAddress(const Wstring& ip);

private:

	SOCKADDR_IN		mSockAddr;
};

