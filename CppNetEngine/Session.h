#pragma once
#include "IocpCore.h"
#include "LockFreeQueue.h"
#include "NetAddress.h"
#include "ReceiveBuffer.h"
#include "SharedPtrUtils.h"

class Session : public IocpObject
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
	virtual void Dispatch(class IocpEvent& iocpEvent, int32 numOfBytes) override;

	void SetNetAddress(const NetAddress& address);

	SOCKET GetSocket() const;
	NetAddress& GetAddress();
	ReceiveBuffer& GetReceiveBuffer();

private:

	SOCKET mSocket;
	NetAddress mNetAddress;
	std::atomic<bool> mbConnected;
	ReceiveBuffer mReceiveBuffer;
	LockFreeQueue<SendBufferRef> mSendQueue;		// 전송할 버퍼
	LockFreeQueue<SendBufferRef> mSendPendingQueue; // 전송중인 버퍼
};

