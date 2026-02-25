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

	Service(const eServiceType serviceType, const NetAddress& netAddress, IocpCoreRef pIocpCore, JobSchedulerRef pScheduler, SessionFactory pSessionFactory, const int32 maxEnterWaitQueueCount, const int32 maxSessionCount = 1);
	virtual ~Service() = default;

	virtual bool Start() = 0;
	virtual void CloseService();

	SessionRef CreateSession();
	void AddSession(const SessionRef& pSession);
	void ReleaseSession(const SessionRef& pSession);
	bool EnterWaitQueue(const SessionRef& pSession);
	bool DequeueWaitQueue(const int32 index);

	eServiceType GetServiceType() const;
	NetAddress& GetNetAddress();
	IocpCoreRef GetIocpCore() const;
	JobSchedulerRef GetJobScheduler() const;
	int32 GetCurrentSessionCount() const;
	int32 GetMaxSessionCount() const;

private:

	const eServiceType mServiceType;
	const int32	mMaxSessionCount;
	NetAddress mNetAddress;
	IocpCoreRef mpIocpCore;
	JobSchedulerRef mpScheduler;
	SessionFactory mpSessionFactory;

	Vector<SessionRef> mSessions;
	LockFreeStack<int32> mReleaseSessionIndexStack;
	LockFreeQueue<WeakSessionRef> mEnterWaitQueue;
};

class ClientService : public Service
{
public:
	ClientService(const NetAddress& targetAddress, IocpCoreRef pIocpCore, JobSchedulerRef pScheduler, SessionFactory pSessionFactory, const int32 maxEnterWaitQueueCount, const int32 maxSessionCount = 1);
	virtual ~ClientService() override = default;

	virtual bool Start() override;
	virtual void CloseService() override;
};

class ServerService : public Service
{
public:
	ServerService(const NetAddress& targetAddress, IocpCoreRef pIocpCore, JobSchedulerRef pScheduler, SessionFactory pSessionFactory, const int32 maxEnterWaitQueueCount, const int32 maxSessionCount = 1);
	virtual ~ServerService() override = default;

	virtual bool Start() override;
	virtual void CloseService() override;

private:
	ListenerRef		mListener;
};