#pragma once

class IiocpObject
{
public:

	IiocpObject(const IiocpObject&) = delete;
	IiocpObject& operator=(const IiocpObject&) = delete;
	IiocpObject(IiocpObject&&) = delete;
	IiocpObject& operator=(IiocpObject&&) = delete;

	IiocpObject();
	virtual ~IiocpObject() = 0;

	[[nodiscard]]
	virtual HANDLE GetHandle() const = 0;
	virtual void Dispatch(class IocpEvent& iocpEvent, int32 numOfBytes) = 0;
};

class IocpCore final
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
	bool Register(IiocpObject& iocpObject) const;

	[[nodiscard]]
	bool Dispatch(int32& outErrorCode, const uint32 timeout = INFINITE) const;

private:

	const HANDLE mIocpHandle;
};

