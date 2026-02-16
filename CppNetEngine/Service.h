#pragma once

#include <functional>

#include "NetAddress.h"
#include "LockFreeStack.h"

enum class eServiceType : uint8
{
	Server,
	Client
};

using SessionFactory = std::function<SessionRef()>;

class Service : public std::enable_shared_from_this<Service>
{
public:

	Service(const Service&) = delete;
	Service& operator=(const Service&) = delete;
	Service(Service&&) = delete;
	Service& operator=(Service&&) = delete;

	Service(eServiceType serviceType, const NetAddress& netAddress, IocpCoreRef pIocpCore, SessionFactory pSessionFactory, int32 maxSessionCount = 1);
	virtual ~Service() = default;

	virtual bool Start() = 0;
	virtual void CloseService();

	SessionRef CreateSession();
	bool AddSession(const SessionRef& pSession);
	void ReleaseSession(const SessionRef& pSession);

	eServiceType GetServiceType() const;
	NetAddress& GetNetAddress();
	IocpCoreRef GetIocpCore() const;
	int32 GetCurrentSessionCount() const;
	int32 GetMaxSessionCount() const;

private:

	const eServiceType mServiceType;
	const int32	mMaxSessionCount;
	NetAddress mNetAddress;
	IocpCoreRef mpIocpCore;

	Vector<SessionRef> mSessions;
	LockFreeStack<int32> mReleaseSessionIndexStack;
	SessionFactory mpSessionFactory;
};

class ClientService : public Service
{
public:
	ClientService(const NetAddress& targetAddress, IocpCoreRef core, SessionFactory factory, const int32 maxSessionCount = 1);
	virtual ~ClientService() override = default;
};

class ServerService : public Service
{
public:
	ServerService(const NetAddress& targetAddress, IocpCoreRef core, SessionFactory factory, const int32 maxSessionCount = 1);
	virtual ~ServerService() override = default;

private:
	ListenerRef		mListener;
};