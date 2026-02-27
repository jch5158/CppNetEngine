#include "pch.h"
#include "Service.h"
#include "Listener.h"
#include "Session.h"
#include "SessionReaper.h"
#include <utility>

Service::Service(const eServiceType serviceType, const NetAddress& netAddress, IocpCoreRef pIocpCore,
                 JobSchedulerRef pScheduler, SessionFactory pSessionFactory, SessionManagerRef pSessionManager, WaitQueueManagerRef pWaitQueueManager)
	: mServiceType(serviceType)
	, mMaxSessionCount(pSessionManager->GetMaxSessionCount())
	, mNetAddress(netAddress)
	, mpIocpCore(std::move(pIocpCore))
	, mpScheduler(std::move(pScheduler))
	, mpSessionFactory(std::move(pSessionFactory))
	, mpSessionManager(std::move(pSessionManager))
	, mpSessionReaper()
	, mpWaitQueueManager(std::move(pWaitQueueManager))
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

bool Service::AddSession(const SessionRef& pSession) const
{
	return mpSessionManager->AddSession(pSession);
}

void Service::ReleaseSession(const SessionRef& pSession) const
{
	pSession->OnDisconnected();

	mpSessionManager->ReleaseSession(pSession);
}

int32 Service::EnterWaitQueue(const SessionRef& pSession) const
{
	if (mpWaitQueueManager == nullptr)
	{
		return -1;
	}

	return mpWaitQueueManager->EnterWaitQueue(pSession);
}

SessionRef Service::DequeueWaitQueue() const
{
	if (mpWaitQueueManager == nullptr)
	{
		return nullptr;
	}

	return mpWaitQueueManager->DequeueWaitQueue();
}

void Service::RegisterSessionReap(const SessionRef& pSession) const
{
	const WeakSessionRef pWeak = pSession;

	mpSessionReaper->DoTimer(mpScheduler, pSession->OnGetTimeoutMs(),
		[pWeak]()->void
		{
			SessionReaper::ReapSession(pWeak);
		});
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
	return mpSessionManager->GetCurrentSessionCount();
}

int32 Service::GetMaxSessionCount() const
{
	return mMaxSessionCount;
}

int32 Service::GetWaitCount(const int32 myWaitTicket) const
{
	if (mpWaitQueueManager == nullptr)
	{
		return -1;
	}

	return mpWaitQueueManager->GetWaitCount(myWaitTicket);
}

ClientService::ClientService(const NetAddress& targetAddress, IocpCoreRef pIocpCore, JobSchedulerRef pScheduler, SessionFactory pSessionFactory, SessionManagerRef pSessionManager)
	: Service(eServiceType::Client, targetAddress, std::move(pIocpCore), std::move(pScheduler), std::move(pSessionFactory), std::move(pSessionManager), nullptr)
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

ServerService::ServerService(const NetAddress& targetAddress, IocpCoreRef pIocpCore, JobSchedulerRef pScheduler, SessionFactory pSessionFactory, SessionManagerRef pSessionManager, WaitQueueManagerRef pWaitQueueManager)
	: Service(eServiceType::Server, targetAddress, std::move(pIocpCore), std::move(pScheduler), std::move(pSessionFactory), std::move(pSessionManager), std::move(pWaitQueueManager))
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
