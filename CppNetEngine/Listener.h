#pragma once
#include "IocpCore.h"
#include "StlAllocator.h"
#include "UniquePtrUtils.h"

class NetAddress;
class IocpAcceptEvent;

class Listener : public IiocpObject
{
public:

	Listener(const Listener&) = delete;
	Listener& operator=(const Listener&) = delete;
	Listener(Listener&&) = delete;
	Listener& operator=(Listener&&) = delete;

	explicit Listener();
	virtual ~Listener() override;

	[[nodiscard]]
	virtual HANDLE GetHandle() const override;
	virtual void Dispatch(class IocpEvent& iocpEvent, int32 numOfBytes) override;

	bool StartAccept(const NetAddress& netAddress);
	bool CloseAccept();

private:

	void registerAccept(IocpAcceptEvent* pAcceptEvent);
	void processAccept(IocpAcceptEvent* pAcceptEvent);

	SOCKET mSocket;
	Vector<UniquePtr<IocpAcceptEvent>> mAcceptEvents;
};

