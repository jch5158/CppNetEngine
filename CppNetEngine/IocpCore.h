#pragma once

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

	explicit IocpCore();
	~IocpCore();

	[[nodiscard]]
	HANDLE GetHandle() const;

	[[nodiscard]]
	bool Register(const IocpObjectRef& iocpObject) const;

	[[nodiscard]]
	bool Dispatch(int32& outErrorCode, const uint32 timeout = INFINITE) const;

private:

	HANDLE mIocpHandle;
};

