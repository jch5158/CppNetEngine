#include "pch.h"
#include "IocpEvent.h"
#include "IocpCore.h"
#include "CrashReporter.h"


IocpCore::IocpCore()
	:mIocpHandle(nullptr)
{
	mIocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (mIocpHandle == nullptr)
	{
		CrashReporter::Crash();
	}
}

IocpCore::~IocpCore()
{
	if (mIocpHandle != nullptr)
	{
		CloseHandle(mIocpHandle);
	}
}

HANDLE IocpCore::GetHandle() const
{
	return mIocpHandle;
}

bool IocpCore::Register(const IocpObjectRef& iocpObject) const
{
	if (nullptr == CreateIoCompletionPort(iocpObject->GetHandle(), mIocpHandle, 0, 0))
	{
		return false;
	}

	return true;
}

bool IocpCore::Dispatch(int32& outErrorCode, const uint32 timeout) const
{
	uint32 numOfBytes = 0;
	IocpObject* pIocpObject = nullptr;
	IocpEvent* pIocpEvent = nullptr;
	const int32 gqcsRet = GetQueuedCompletionStatus(mIocpHandle, reinterpret_cast<LPDWORD>(&numOfBytes), reinterpret_cast<PULONG_PTR>(&pIocpObject), reinterpret_cast<LPOVERLAPPED*>(&pIocpEvent), timeout);
	if (gqcsRet != 0)
	{
		pIocpObject->Dispatch(*pIocpEvent, static_cast<int32>(numOfBytes));
	}
	else
	{
		outErrorCode = WSAGetLastError();
		return false;
	}

	return true;
}
