#pragma once
#include "NetAddress.h"

class SocketUtils final  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:

	SocketUtils() = delete;
	~SocketUtils() = delete;
	SocketUtils(const SocketUtils&) = delete;
	SocketUtils& operator=(const SocketUtils&) = delete;
	SocketUtils(SocketUtils&&) = delete;
	SocketUtils& operator=(SocketUtils&&) = delete;

	static bool Init(int32& outErrorCode);
	static void Clear();
	static bool CreateTcpSocket(SOCKET& outSocket);
	static void Close(SOCKET& socket);
	static bool SetLinger(const SOCKET socket, const uint16 onOff, const uint16 linger);
	static bool SetReuseAddress(SOCKET socket, bool flag);
	static bool SetRecvBufferSize(SOCKET socket, int32 size);
	static bool SetSendBufferSize(SOCKET socket, int32 size);
	static bool SetTcpNoDelay(SOCKET socket, bool flag);
	static bool SetUpdateAcceptSocket(const SOCKET socket, const SOCKET listenSocket);
	static bool Bind(const SOCKET socket, const NetAddress netAddr);
	static bool BindAnyAddress(const SOCKET socket, const uint16 port);
	static bool Listen(const SOCKET socket, const int32 backlog);

private:

	static bool bindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);

	static LPFN_CONNECTEX connectEx;
	static LPFN_DISCONNECTEX disconnectEx;
	static LPFN_ACCEPTEX acceptEx;
};

template<typename T>
static inline bool SetSockOpt(const SOCKET socket, const int32 level, const int32 optName, T optVal)
{
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
}

