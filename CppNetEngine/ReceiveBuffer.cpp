#include "pch.h"
#include "ReceiveBuffer.h"

ReceiveBuffer::ReceiveBuffer()
	: mFront(0)
	, mRear(0)
	, mBufferSize(DEFAULT_BUFFER_SIZE)
	, mBuffer(static_cast<char*>(MemoryAllocator::GetInstance().Alloc(DEFAULT_BUFFER_SIZE)), BufferDeleter(DEFAULT_BUFFER_SIZE))
{
}

ReceiveBuffer::ReceiveBuffer(const int32 bufferSize)
	: mFront(0)
	, mRear(0)
	, mBufferSize(bufferSize)
	, mBuffer(static_cast<char*>(MemoryAllocator::GetInstance().Alloc(bufferSize)), BufferDeleter(bufferSize))
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

char* ReceiveBuffer::GetReadBuffer() const
{
	return &mBuffer[mFront];
}

char* ReceiveBuffer::GetWriteBuffer() const
{
	return &mBuffer[mRear];
}

int32 ReceiveBuffer::GetUseSize() const
{
	const int32 front = mFront;
	const int32 rear = mRear;
	int32 useSize;

	if (front > rear)
	{
		useSize = mBufferSize - (front - rear);
	}
	else
	{
		useSize = rear - front;
	}

	return useSize;
}

int32 ReceiveBuffer::GetFreeSize() const
{
	const int32 front = mFront;
	const int32 rear = mRear;
	int32 freeSize;

	if (front > rear)
	{
		freeSize = front - rear - 1;
	}
	else
	{
		freeSize = mBufferSize - (rear - front) - 1;
	}

	return freeSize;
}

int32 ReceiveBuffer::GetDirectWriteSize() const
{
	const int32 front = mFront;
	const int32 rear = mRear;
	int32 writeSize;

	if (front > rear)
	{
		writeSize = front - rear - 1;
	}
	else
	{
		writeSize = mBufferSize - rear - (front == 0 ? 1 : 0);
	}

	return writeSize;
}

int32 ReceiveBuffer::GetDirectReadSize() const
{
	const int32 front = mFront;
	const int32 rear = mRear;
	int32 readSize;

	if (front > rear)
	{
		readSize = mBufferSize - front;
	}
	else
	{
		readSize = rear - front;
	}

	return readSize;
}

bool ReceiveBuffer::IsEmpty() const
{
	return mFront == mRear;
}

int32 ReceiveBuffer::Write(const char* pData, const int32 size)
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

int32 ReceiveBuffer::Read(char* pBuffer, const int32 size)
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

int32 ReceiveBuffer::Peek(char* pBuffer, const int32 size) const
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

