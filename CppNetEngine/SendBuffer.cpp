#include "pch.h"
#include "SendBuffer.h"

SendBuffer::SendBuffer()
	: mWritePos(0)
	, mBufferSize(DEFAULT_BUFFER_SIZE)
	, mBuffer(UniquePtrUtils<byte[]>::Alloc(DEFAULT_BUFFER_SIZE))
{
}

SendBuffer::SendBuffer(const int32 bufferSize)
	: mWritePos(0)
	, mBufferSize(bufferSize)
	, mBuffer(UniquePtrUtils<byte[]>::Alloc(bufferSize))
{
}

byte* SendBuffer::Reserve(const int32 size) const
{
	if (FreeSize() <= size)
	{
		return nullptr;
	}

	return &mBuffer[mWritePos];
}

void SendBuffer::Commit(const int32 size)
{
	if (size <= 0 || FreeSize() <= size)
	{
		return;
	}

	mWritePos += size;
}

int32 SendBuffer::FreeSize() const
{
	return mBufferSize - mWritePos;
}

int32 SendBuffer::UseSize() const
{
	return mWritePos;
}

int32 SendBuffer::MaxSize() const
{
	return mBufferSize;
}

byte* SendBuffer::GetBufferPtr() const
{
	return &mBuffer[0];
}
