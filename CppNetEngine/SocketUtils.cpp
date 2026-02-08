#include "pch.h"
#include "SocketUtils.h"
#include "NetAddress.h"

bool SocketUtils::Init(int32& outErrorCode)
{
	WSADATA wsaData{};
	if (WSAStartup(WINSOCK_VERSION, &wsaData) == SOCKET_ERROR)
	{
		outErrorCode = WSAGetLastError();
		return false;
	}

	SOCKET dummySocket;
	/* 런타임에 주소 얻어오는 API */
	if (CreateTcpSocket(dummySocket) == false)
	{
		outErrorCode = WSAGetLastError();
		return false;
	}

	if (bindWindowsFunction(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&connectEx)) == false)
	{
		outErrorCode = WSAGetLastError();
		return false;
	}

	if (bindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&disconnectEx)) == false)
	{
		outErrorCode = WSAGetLastError();
		return false;
	}

	if (bindWindowsFunction(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&acceptEx)) == false)
	{
		outErrorCode = WSAGetLastError();
		return false;
	}
	
	Close(dummySocket);

	return true;
}

void SocketUtils::Clear()
{
	WSACleanup();
}

bool SocketUtils::CreateTcpSocket(SOCKET& outSocket)
{
	outSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (std::cmp_equal(outSocket, INVALID_SOCKET))
	{
		return false;
	}

	return true;
}

void SocketUtils::Close(SOCKET& socket)
{
	if (std::cmp_equal(socket, INVALID_SOCKET))
	{
		closesocket(socket);
	}

	socket = INVALID_SOCKET;
}

bool SocketUtils::SetLinger(const SOCKET socket, const uint16 onOff, const uint16 linger)
{
	LINGER option;
	option.l_onoff = onOff;
	option.l_linger = linger;
	return SetSockOpt(socket, SOL_SOCKET, SO_LINGER, option);
}

bool SocketUtils::SetReuseAddress(const SOCKET socket, const bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_REUSEADDR, flag);
}

bool SocketUtils::SetRecvBufferSize(const SOCKET socket, const int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
}

bool SocketUtils::SetSendBufferSize(const SOCKET socket, const int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
}

bool SocketUtils::SetTcpNoDelay(const SOCKET socket, const bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, TCP_NODELAY, flag);
}

// ListenSocket의 특성을 ClientSocket에 그대로 적용
bool SocketUtils::SetUpdateAcceptSocket(const SOCKET socket, const SOCKET listenSocket)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}

bool SocketUtils::Bind(const SOCKET socket, const NetAddress netAddr)
{
	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&netAddr.GetSockAddr()), sizeof(SOCKADDR_IN));
}

bool SocketUtils::BindAnyAddress(const SOCKET socket, const uint16 port)
{
	SOCKADDR_IN myAddress;
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
	myAddress.sin_port = ::htons(port);

	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&myAddress), sizeof(myAddress));
}

bool SocketUtils::Listen(const SOCKET socket, const int32 backlog)
{
	return SOCKET_ERROR != ::listen(socket, backlog);
}

bool SocketUtils::bindWindowsFunction(const SOCKET socket, GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), static_cast<LPVOID*>(fn), sizeof(*fn), OUT & bytes, nullptr, nullptr);
}

LPFN_CONNECTEX SocketUtils::connectEx = nullptr;
LPFN_DISCONNECTEX SocketUtils::disconnectEx = nullptr;
LPFN_ACCEPTEX SocketUtils::acceptEx = nullptr;