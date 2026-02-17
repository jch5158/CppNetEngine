#pragma once

#include "Types.h"
#include "INetBuffer.h"
#include "StlTypes.h"

template <int32 BUFFER_SIZE>
class NetSendBuffer final : public INetBuffer
{
public:

	NetSendBuffer(const NetSendBuffer&) = delete;
	NetSendBuffer& operator=(const NetSendBuffer&) = delete;
	NetSendBuffer(NetSendBuffer&&) = delete;
	NetSendBuffer& operator=(NetSendBuffer&&) = delete;

	explicit NetSendBuffer()
		: mReadPos(0)
		, mWritePos(0)
		, mBuffer()
	{
		ASSERT(BUFFER_SIZE <= MAX_BUFFER_SIZE, "BUFFER_SIZE is equal to or less than MAX_BUFFER_SIZE");
	}

	virtual ~NetSendBuffer() override = default;
	
	virtual void Clear() override
	{
		mReadPos = 0;
		mWritePos = 0;
	}

	[[nodiscard]]
	virtual int32 GetMaxSize() const override
	{
		return BUFFER_SIZE;
	}

	[[nodiscard]]
	virtual int32 GetFreeSize() const override
	{
		return BUFFER_SIZE - mWritePos;
	}

	[[nodiscard]]
	virtual int32 GetUseSize() const override
	{
		return mWritePos;
	}

	[[nodiscard]]
	virtual byte* GetBufferPtr() override
	{
		return mBuffer.data();
	}

	[[nodiscard]]
	virtual byte* GetReadPtr() override
	{
		return &mBuffer[mReadPos];
	}

	[[nodiscard]]
	virtual byte* GetWritePtr() override
	{
		return &mBuffer[mWritePos];
	}

	virtual void MoveReadPos(const int32 size) override
	{
		if (size + mReadPos > mWritePos)
		{
			return;
		}

		mReadPos += size;
	}

	virtual void MoveWritePos(const int32 size) override
	{
		if (std::cmp_less(GetFreeSize(), size))
		{
			return;
		}

		mWritePos += size;
	}

	[[nodiscard]]
	byte* Reserve(const int32 size) const
	{
		if (std::cmp_less(GetFreeSize(), size))
		{
			return nullptr;
		}

		return &mBuffer[mWritePos];
	}

	void Commit(const int32 size)
	{
		MoveWritePos(size);
	}

private:
	int32 mReadPos;
	int32 mWritePos;
	Array<byte, BUFFER_SIZE> mBuffer;
};