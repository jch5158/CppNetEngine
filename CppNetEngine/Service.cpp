#include "pch.h"
#include "Service.h"
#include "Listener.h"
#include "Session.h"
#include <utility>

Service::Service(const eServiceType serviceType, const NetAddress& netAddress, IocpCoreRef pIocpCore,
                 JobSchedulerRef pScheduler, SessionFactory pSessionFactory, const int32 maxEnterWaitQueueCount, const int32 maxSessionCount)
	: mServiceType(serviceType)
	, mMaxSessionCount(maxSessionCount)
	, mNetAddress(netAddress)
	, mpIocpCore(std::move(pIocpCore))
	, mpScheduler(std::move(pScheduler))
	, mpSessionFactory(std::move(pSessionFactory))
	, mSessionManager(maxSessionCount)
	, mWaitQueueManager(maxEnterWaitQueueCount)
{
	if (mpSessionFactory == nullptr)
	{
		NET_ENGINE_LOG_FATAL("Service::Service - mSessionFactory is nullptr");
		CrashReporter::Crash();
	}
}

void Service::CloseService()
{
	// TODO : CloseService
}

SessionRef Service::CreateSession()
{
	SessionRef session = mpSessionFactory();
	session->setService(shared_from_this());

	if (mpIocpCore->Register(session) == false)
	{
		session = nullptr;
	}

	return session;
}

bool Service::AddSession(const SessionRef& pSession)
{
	return mSessionManager.AddSession(pSession);
}

int32 Service::ReleaseSession(const SessionRef& pSession)
{
	pSession->OnDisconnected();

	const int32 sessionIndex = pSession->GetSessionIndex();
	pSession->setSessionIndex(-1);
	mSessionManager.ReleaseSession(sessionIndex);

	return sessionIndex;
}

void Service::ReleaseSessionIndex(const int32 index)
{
	mSessionManager.ReleaseSessionIndex(index);
}

int32 Service::EnterWaitQueue(const SessionRef& pSession)
{
	return mWaitQueueManager.EnterWaitQueue(pSession);
}

SessionRef Service::DequeueWaitQueue()
{
	return mWaitQueueManager.DequeueWaitQueue();
}

eServiceType Service::GetServiceType() const
{
	return mServiceType;
}

NetAddress& Service::GetNetAddress()
{
	return mNetAddress;
}

IocpCoreRef Service::GetIocpCore() const
{
	return mpIocpCore;
}

JobSchedulerRef Service::GetJobScheduler() const
{
	return mpScheduler;
}

int32 Service::GetCurrentSessionCount() const
{
	return mSessionManager.GetCurrentSessionCount();
}

int32 Service::GetMaxSessionCount() const
{
	return mMaxSessionCount;
}

int32 Service::GetWaitCount(const int32 myWaitTicket) const
{
	return mWaitQueueManager.GetWaitCount(myWaitTicket);
}

ClientService::ClientService(const NetAddress& targetAddress, IocpCoreRef pIocpCore, JobSchedulerRef pScheduler, SessionFactory pSessionFactory, const int32 maxEnterWaitQueueCount, const int32 maxSessionCount)
	: Service(eServiceType::Client, targetAddress, std::move(pIocpCore), std::move(pScheduler), std::move(pSessionFactory), maxEnterWaitQueueCount, maxSessionCount)
{
}

bool ClientService::Start()
{
	const int32 sessionCount = GetMaxSessionCount();
	for (int32 i = 0; i < sessionCount; i++)
	{
		const SessionRef pSession = CreateSession();
		if (pSession->Connect() == false)
		{
			return false;
		}
	}

	return true;
}

void ClientService::CloseService()
{
}

ServerService::ServerService(const NetAddress& targetAddress, IocpCoreRef pIocpCore, JobSchedulerRef pScheduler, SessionFactory pSessionFactory, const int32 maxEnterWaitQueueCount, const int32 maxSessionCount)
	: Service(eServiceType::Server, targetAddress, std::move(pIocpCore), std::move(pScheduler), std::move(pSessionFactory), maxEnterWaitQueueCount, maxSessionCount)
{
}

bool ServerService::Start()
{
	mListener = cpp_net_engine::MakeShared<Listener>();
	if (mListener == nullptr)
	{
		return false;
	}

	const ServerServiceRef service = static_pointer_cast<ServerService>(shared_from_this());
	if (mListener->StartAccept(service) == false)
	{
		return false;
	}

	return true;
}

void ServerService::CloseService()
{
}
