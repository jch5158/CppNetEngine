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
	, mSessions(maxSessionCount, nullptr)
	, mReleaseSessionIndexStack(maxSessionCount)
	, mWaitQueueCount(0)
	, mEnterWaitQueue(maxEnterWaitQueueCount)
{
	if (mpSessionFactory == nullptr)
	{
		NET_ENGINE_LOG_FATAL("Service::Service - mSessionFactory is nullptr");
		CrashReporter::Crash();
	}

	for (int32 i = 0; i < mMaxSessionCount; ++i)
	{
		if (!mReleaseSessionIndexStack.TryPush(i))
		{
			NET_ENGINE_LOG_FATAL("Service::Service - mReleaseSessionIndexStack is full");
			CrashReporter::Crash();
		}
	}
}

void Service::CloseService()
{
	// TODO : CloseService
}

SessionRef Service::CreateSession()
{
	SessionRef session = mpSessionFactory();
	session->SetService(shared_from_this());

	if (mpIocpCore->Register(session) == false)
	{
		session = nullptr;
	}

	return session;
}

bool Service::AddSession(const SessionRef& pSession)
{
	int32 sessionIndex = -1;
	if (mReleaseSessionIndexStack.TryPop(sessionIndex) == false)
	{
		return false;
	}

	pSession->SetSessionIndex(sessionIndex);
	mSessions[sessionIndex] = pSession;

	return true;
}

int32 Service::ReleaseSession(const SessionRef& pSession)
{
	pSession->OnDisconnected();

	const int32 sessionIndex = pSession->GetSessionIndex();
	pSession->SetSessionIndex(-1);
	mSessions[sessionIndex] = nullptr;

	return sessionIndex;
}

void Service::ReleaseSessionIndex(const int32 index)
{
	if (mReleaseSessionIndexStack.TryPush(index) == false)
	{
		NET_ENGINE_LOG_FATAL("Service::ReleaseSession - mReleaseSessionIndexStack is full");
	}
}

bool Service::EnterWaitQueue(const SessionRef& pSession)
{
	const int32 waitCount = mWaitQueueCount.fetch_add(1);

	pSession->SetWaitTicket(waitCount);

	return mEnterWaitQueue.TryEnqueue(pSession);
}

bool Service::DequeueWaitQueue(const int32 index)
{
	while (!mEnterWaitQueue.IsEmpty())
	{
		WeakSessionRef pWeakSession;
		if (mEnterWaitQueue.TryDequeue(pWeakSession) == true)
		{
			SessionRef pSession = pWeakSession.lock();
			if (pSession == nullptr)
			{
				continue;
			}

			return true;
		}
	}

	return false;
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
	return mMaxSessionCount - mReleaseSessionIndexStack.Count();
}

int32 Service::GetMaxSessionCount() const
{
	return mMaxSessionCount;
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
