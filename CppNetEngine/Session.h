#pragma once
#include "IocpCore.h"
#include "NetAddress.h"
#include "ReceiveBuffer.h"

class ReceiveBuffer;

class Session : public IiocpObject
{
public:

	Session(const Session&) = delete;
	Session& operator=(const Session&) = delete;
	Session(Session&&) = delete;
	Session& operator=(Session&&) = delete;

	Session();
	virtual ~Session() override;

	[[nodiscard]]
	virtual HANDLE GetHandle() const override;
	virtual void Dispatch(class IocpEvent& iocpEvent, int32 numOfBytes);

	void SetNetAddress(const NetAddress& address);

	SOCKET GetSocket() const;
	NetAddress& GetAddress();
	ReceiveBuffer& GetReceiveBuffer();

private:

	SOCKET mSocket;
	NetAddress mNetAddress;
	std::atomic<bool> mConnected;
	ReceiveBuffer mReceiveBuffer;
};

