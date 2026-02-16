#include "pch.h"
#include "ReceiveBuffer.h"
#include "MemoryAllocator.h"
#include "UniquePtrUtils.h"

ReceiveBuffer::ReceiveBuffer()
	: mReadPos(0)
	, mWritePos(0)
	, mBufferSize(DEFAULT_BUFFER_SIZE)
	, mBuffer(UniquePtrUtils<byte[]>::Alloc(DEFAULT_BUFFER_SIZE))
{
}

ReceiveBuffer::ReceiveBuffer(const int32 bufferSize)
	: mReadPos(0)
	, mWritePos(0)
	, mBufferSize(bufferSize)
	, mBuffer(UniquePtrUtils<byte[]>::Alloc(bufferSize))
{
}

void ReceiveBuffer::MoveWritePos(const int32 size)
{
	mWritePos = (mWritePos + size) % mBufferSize;
}

void ReceiveBuffer::MoveReadPos(const int32 size)
{
	mReadPos = (mReadPos + size) % mBufferSize;
}

void ReceiveBuffer::Clear()
{
	mReadPos = 0;
	mWritePos = 0;
}

byte* ReceiveBuffer::GetBufferPtr() const
{
	return &mBuffer[0];
}

int32 ReceiveBuffer::GetUseSize() const
{
	const int32 readPos = mReadPos;
	const int32 writePos = mWritePos;
	int32 useSize;

	if (readPos > writePos)
	{
		useSize = mBufferSize - (readPos - writePos);
	}
	else
	{
		useSize = writePos - readPos;
	}

	return useSize;
}

int32 ReceiveBuffer::GetFreeSize() const
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
		freeSize = mBufferSize - (writePos - readPos) - 1;
	}

	return freeSize;
}

byte* ReceiveBuffer::GetReadPointer() const
{
	return &mBuffer[mReadPos];
}

byte* ReceiveBuffer::GetWritePointer() const
{
	return &mBuffer[mWritePos];
}

int32 ReceiveBuffer::GetLinearWriteSize() const
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
		writeSize = mBufferSize - writePos - (readPos == 0 ? 1 : 0);
	}

	return writeSize;
}

int32 ReceiveBuffer::GetLinearReadSize() const
{
	const int32 readPos = mReadPos;
	const int32 writePos = mWritePos;
	int32 readSize;

	if (readPos > writePos)
	{
		readSize = mBufferSize - readPos;
	}
	else
	{
		readSize = writePos - readPos;
	}

	return readSize;
}

bool ReceiveBuffer::IsEmpty() const
{
	return mReadPos == mWritePos;
}

int32 ReceiveBuffer::Write(const byte* pData, const int32 size)
{
	const int32 writeSize = std::min(GetFreeSize(), size);
	if (writeSize == 0)
	{
		return writeSize;
	}

	if (writeSize + mWritePos < mBufferSize)
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

int32 ReceiveBuffer::Read(byte* pBuffer, const int32 size)
{
	const int32 readSize = std::min(GetUseSize(), size);
	if (readSize == 0)
	{
		return readSize;
	}

	if (readSize + mReadPos > mBufferSize)
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

int32 ReceiveBuffer::Peek(byte* pBuffer, const int32 size) const
{
	const int32 readSize = std::min(GetUseSize(), size);
	if (readSize == 0)
	{
		return readSize;
	}

	if (readSize + mReadPos > mBufferSize)
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
