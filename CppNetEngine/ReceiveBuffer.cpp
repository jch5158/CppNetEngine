#include "pch.h"
#include "ReceiveBuffer.h"

ReceiveBuffer::ReceiveBuffer()
	: mFront(0)
	, mRear(0)
	, mBufferSize(DEFAULT_BUFFER_SIZE)
	, mBuffer(static_cast<byte*>(MemoryAllocator::GetInstance().Alloc(DEFAULT_BUFFER_SIZE)), BufferDeleter(DEFAULT_BUFFER_SIZE))
{
}

ReceiveBuffer::ReceiveBuffer(const int32 bufferSize)
	: mFront(0)
	, mRear(0)
	, mBufferSize(bufferSize)
	, mBuffer(static_cast<byte*>(MemoryAllocator::GetInstance().Alloc(bufferSize)), BufferDeleter(bufferSize))
{
}

void ReceiveBuffer::MoveRear(const int32 size)
{
	mRear = (mRear + size) % mBufferSize;
}

void ReceiveBuffer::MoveFront(const int32 size)
{
	mFront = (mFront + size) % mBufferSize;
}

void ReceiveBuffer::Clear()
{
	mFront = 0;
	mRear = 0;
}

byte* ReceiveBuffer::GetReadBuffer() const
{
	return &mBuffer[mFront];
}

byte* ReceiveBuffer::GetWriteBuffer() const
{
	return &mBuffer[mRear];
}

int32 ReceiveBuffer::GetUseSize() const
{
	int32 useSize;

	if (mFront > mRear)
	{
		useSize = mBufferSize - (mFront - mRear);
	}
	else
	{
		useSize = mRear - mFront;
	}

	return useSize;
}

int32 ReceiveBuffer::GetFreeSize() const
{
	int freeSize;

	if (mFront > mRear)
	{
		freeSize = mFront - mRear - 1;
	}
	else
	{
		freeSize = mBufferSize - (mRear - mFront) - 1;
	}

	return freeSize;
}

int32 ReceiveBuffer::GetDirectWriteSize() const
{
	int32 writeSize;

	if (mFront > mRear)
	{
		writeSize = mFront - mRear - 1;
	}
	else
	{
		writeSize = mBufferSize - mRear - (mFront == 0 ? 1 : 0);
	}

	return writeSize;
}

int32 ReceiveBuffer::GetDirectReadSize() const
{
	int32 readSize;

	if (mFront > mRear)
	{
		readSize = mBufferSize - mFront;
	}
	else
	{
		readSize = mRear - mFront;
	}

	return readSize;
}

bool ReceiveBuffer::IsEmpty() const
{
	return mFront == mRear;
}

int32 ReceiveBuffer::Write(const byte* pData, const int32 size)
{
	const int32 writeSize = std::min(GetFreeSize(), size);
	if (writeSize == 0)
	{
		return writeSize;
	}

	if (writeSize + mRear < mBufferSize)
	{
		std::copy_n(pData, writeSize, &mBuffer[mRear]);
	}
	else
	{
		const int32 directSize = GetDirectWriteSize();
		std::copy_n(pData, directSize, &mBuffer[mRear]);

		const int32 remainSize = writeSize - directSize;
		std::copy_n(&pData[directSize], remainSize, &mBuffer[0]);
	}

	MoveRear(writeSize);

	return writeSize;
}

int32 ReceiveBuffer::Read(byte* pBuffer, const int32 size)
{
	const int32 readSize = std::min(GetUseSize(), size);
	if (readSize == 0)
	{
		return readSize;
	}

	if (readSize + mFront <= mBufferSize)
	{
		std::copy_n(&mBuffer[mFront], readSize, pBuffer);
	}
	else
	{
		const int32 directSize = GetDirectReadSize();
		std::copy_n(&mBuffer[mFront], directSize, pBuffer);

		const int32 remainSize = readSize - directSize;
		std::copy_n(&mBuffer[0], remainSize, &pBuffer[directSize]);
	}

	MoveFront(readSize);

	return readSize;
}

int32 ReceiveBuffer::Peek(byte* pBuffer, const int32 size) const
{
	const int32 readSize = std::min(GetUseSize(), size);
	if (readSize == 0)
	{
		return readSize;
	}

	if (readSize + mFront <= mBufferSize)
	{
		std::copy_n(&mBuffer[mFront], readSize, pBuffer);
	}
	else
	{
		const int32 directSize = GetDirectReadSize();
		std::copy_n(&mBuffer[mFront], directSize, pBuffer);

		const int32 remainSize = readSize - directSize;
		std::copy_n(&mBuffer[0], remainSize, &pBuffer[directSize]);
	}

	return readSize;
}

