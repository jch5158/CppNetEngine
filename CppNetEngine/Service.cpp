#include "pch.h"
#include "Service.h"
#include <utility>

#include "Session.h"

Service::Service(const eServiceType serviceType, const NetAddress& netAddress, IocpCoreRef pIocpCore, SessionFactory pSessionFactory, const int32 maxSessionCount)
	: mServiceType(serviceType)
	, mMaxSessionCount(maxSessionCount)
	, mNetAddress(netAddress)
	, mpIocpCore(std::move(pIocpCore))
	, mSessions(maxSessionCount, nullptr)
	, mReleaseSessionIndexStack(maxSessionCount)
	, mpSessionFactory(std::move(pSessionFactory))
{
	ASSERT(mpSessionFactory != nullptr, "Service - mSessionFactory is nullptr");

	for (int32 i = 0; i < mMaxSessionCount; ++i)
	{
		ASSERT(mReleaseSessionIndexStack.TryPush(i) != true, "Service - mReleaseSessionIndexStack is full");
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

void Service::ReleaseSession(const SessionRef& pSession)
{
	const int32 sessionIndex = pSession->GetSessionIndex();
	if (sessionIndex == -1)
	{
		return;
	}

	pSession->SetSessionIndex(-1);
	mSessions[sessionIndex] = nullptr;
	if (mReleaseSessionIndexStack.TryPush(sessionIndex) == false)
	{
		ASSERT(false, "Service::ReleaseSession - mReleaseSessionIndexStack is full");
	}
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

int32 Service::GetCurrentSessionCount() const
{
	return mMaxSessionCount - mReleaseSessionIndexStack.Count();
}

int32 Service::GetMaxSessionCount() const
{
	return mMaxSessionCount;
}

ClientService::ClientService(const NetAddress& targetAddress, IocpCoreRef core, SessionFactory factory, const int32 maxSessionCount)
	: Service(eServiceType::Client, targetAddress, std::move(core), std::move(factory), maxSessionCount)
{
}

ServerService::ServerService(const NetAddress& targetAddress, IocpCoreRef core, SessionFactory factory, const int32 maxSessionCount)
	: Service(eServiceType::Server, targetAddress, std::move(core), std::move(factory), maxSessionCount)
{
}
