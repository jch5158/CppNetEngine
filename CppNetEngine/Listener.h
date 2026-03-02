#pragma once
#include "IocpCore.h"
#include "SharedPtrUtils.h"

class IocpAcceptEvent;

class Listener final : public IocpObject
{
public:

	using ErrorHandle = std::function<void(const uint32)>;

	Listener(const Listener&) = delete;
	Listener& operator=(const Listener&) = delete;
	Listener(Listener&&) = delete;
	Listener& operator=(Listener&&) = delete;

	explicit Listener(const int32 acceptCount, ErrorHandle pErrorHandle);
	virtual ~Listener() override;

	[[nodiscard]]
	virtual HANDLE GetHandle() const override;
	virtual void Dispatch(IocpEvent& iocpEvent, uint32 numOfBytes) override;

	bool StartAccept(const ServerServiceRef& pServerService);
	void CloseAccept();

private:

	void registerAccept(IocpAcceptEvent& acceptEvent) const;
	void processAccept(IocpAcceptEvent& acceptEvent) const;

	SOCKET mSocket;
	const int32 mAcceptCount;
	const ErrorHandle mpErrorHandle;
	Vector<IocpAcceptEvent*> mAcceptEvents;
	ServerServiceRef mpServerService;
};

