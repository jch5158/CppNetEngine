#pragma once

#include "Types.h"
#include "INetBuffer.h"
#include <algorithm>

template <int32 BUFFER_SIZE = INetBuffer::MAX_BUFFER_SIZE>
class NetReceiveBuffer final : public INetBuffer
{
public:

	NetReceiveBuffer(const NetReceiveBuffer&) = delete;
	NetReceiveBuffer& operator=(const NetReceiveBuffer&) = delete;
	NetReceiveBuffer(NetReceiveBuffer&&) = delete;
	NetReceiveBuffer& operator=(NetReceiveBuffer&&) = delete;

	explicit NetReceiveBuffer()
		: mReadPos(0)
		, mWritePos(0)
	{
		static_assert(BUFFER_SIZE <= MAX_BUFFER_SIZE, "BUFFER_SIZE is equal to or less than MAX_BUFFER_SIZE");
	}

	virtual ~NetReceiveBuffer() override = default;

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
		const int32 readPos = mReadPos;
		const int32 writePos = mWritePos;
		int32 freeSize;

		if (readPos > writePos)
		{
			freeSize = readPos - writePos - 1;
		}
		else
		{
			freeSize = BUFFER_SIZE - (writePos - readPos) - 1;
		}

		return freeSize;
	}

	[[nodiscard]]
	virtual int32 GetUseSize() const override
	{
		const int32 readPos = mReadPos;
		const int32 writePos = mWritePos;
		int32 useSize;

		if (readPos > writePos)
		{
			useSize = BUFFER_SIZE - (readPos - writePos);
		}
		else
		{
			useSize = writePos - readPos;
		}

		return useSize;
	}

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
		mReadPos = (mReadPos + size) % BUFFER_SIZE;
	}

	virtual void MoveWritePos(const int32 size) override
	{
		mWritePos = (mWritePos + size) % BUFFER_SIZE;
	}

	[[nodiscard]]
	virtual byte* Reserve(const int32 size) override
	{
		if (std::cmp_less(GetLinearWriteSize(), size))
		{
			return nullptr;
		}

		return GetWritePtr();
	}

	virtual void Commit(const int32 size) override
	{
		MoveWritePos(size);
	}

	[[nodiscard]]
	int32 GetLinearWriteSize() const
	{
		const int32 readPos = mReadPos;
		const int32 writePos = mWritePos;
		int32 writeSize;

		if (readPos > writePos)
		{
			writeSize = readPos - writePos - 1;
		}
		else
		{
			writeSize = BUFFER_SIZE - writePos - (readPos == 0 ? 1 : 0);
		}

		return writeSize;
	}

	[[nodiscard]]
	int32 GetLinearReadSize() const
	{
		const int32 readPos = mReadPos;
		const int32 writePos = mWritePos;
		int32 readSize;

		if (readPos > writePos)
		{
			readSize = BUFFER_SIZE - readPos;
		}
		else
		{
			readSize = writePos - readPos;
		}

		return readSize;
	}


	[[nodiscard]]
	bool IsEmpty() const
	{
		return mReadPos == mWritePos;
	}

	int32 Write(const byte* pData, const int32 size)
	{
		const int32 writeSize = std::min(GetFreeSize(), size);
		if (writeSize == 0)
		{
			return writeSize;
		}

		if (writeSize + mWritePos < BUFFER_SIZE)
		{
			std::copy_n(pData, writeSize, &mBuffer[mWritePos]);
		}
		else
		{
			const int32 linearSize = GetLinearWriteSize();
			std::copy_n(pData, linearSize, &mBuffer[mWritePos]);

			const int32 remainSize = writeSize - linearSize;
			std::copy_n(&pData[linearSize], remainSize, &mBuffer[0]);
		}

		MoveWritePos(writeSize);

		return writeSize;
	}

	int32 Read(byte* pBuffer, const int32 size)
	{
		const int32 readSize = std::min(GetUseSize(), size);
		if (readSize == 0)
		{
			return readSize;
		}

		if (readSize + mReadPos > BUFFER_SIZE)
		{
			const int32 linearSize = GetLinearReadSize();
			std::copy_n(&mBuffer[mReadPos], linearSize, pBuffer);

			const int32 remainSize = readSize - linearSize;
			std::copy_n(&mBuffer[0], remainSize, &pBuffer[linearSize]);
		}
		else
		{
			std::copy_n(&mBuffer[mReadPos], readSize, pBuffer);
		}

		MoveReadPos(readSize);

		return readSize;
	}

	int32 Peek(byte* pBuffer, const int32 size) const
	{
		const int32 readSize = std::min(GetUseSize(), size);
		if (readSize == 0)
		{
			return readSize;
		}

		if (readSize + mReadPos > BUFFER_SIZE)
		{
			const int32 linearSize = GetLinearReadSize();
			std::copy_n(&mBuffer[mReadPos], linearSize, pBuffer);

			const int32 remainSize = readSize - linearSize;
			std::copy_n(&mBuffer[0], remainSize, &pBuffer[linearSize]);
		}
		else
		{
			std::copy_n(&mBuffer[mReadPos], readSize, pBuffer);
		}

		return readSize;
	}

private:

	int32 mReadPos;
	int32 mWritePos;
	Array<byte, BUFFER_SIZE> mBuffer;
};

template class NetReceiveBuffer<INetBuffer::MAX_BUFFER_SIZE>;