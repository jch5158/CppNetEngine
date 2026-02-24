#pragma once
#include <functional>

class IocpObject : public std::enable_shared_from_this<IocpObject>
{
public:

	IocpObject(const IocpObject&) = delete;
	IocpObject& operator=(const IocpObject&) = delete;
	IocpObject(IocpObject&&) = delete;
	IocpObject& operator=(IocpObject&&) = delete;

	IocpObject();
	virtual ~IocpObject() = 0;

	[[nodiscard]]
	virtual HANDLE GetHandle() const = 0;
	virtual void Dispatch(class IocpEvent& iocpEvent, const int32 numOfBytes) = 0;
};

class IocpCore
{
public:

	IocpCore(const IocpCore&) = delete;
	IocpCore& operator=(const IocpCore&) = delete;
	IocpCore(IocpCore&&) = delete;
	IocpCore& operator=(IocpCore&&) = delete;

	explicit IocpCore(std::function<void(uint32)>& pOnErrorHandler);
	~IocpCore();

	[[nodiscard]]
	HANDLE GetHandle() const;

	[[nodiscard]]
	bool Register(const IocpObjectRef& iocpObject) const;

	void Dispatch(const uint32 timeout = INFINITE) const;

private:

	HANDLE mIocpHandle;
	std::function<void(const uint32)> mpOnErrorHandler;
};

