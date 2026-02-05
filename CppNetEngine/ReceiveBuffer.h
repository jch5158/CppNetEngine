#pragma once

class ReceiveBuffer
{
private:

	class BufferDeleter
	{
	public:

		BufferDeleter() = delete;
		explicit BufferDeleter(const int32 size)
			:mSize(size)
		{
		}

		void operator()(byte* pBuffer) const
		{
			MemoryAllocator::GetInstance().Free(pBuffer, mSize);
		}

	public:

		const int32 mSize;
	};

public:

	constexpr static int32 DEFAULT_BUFFER_SIZE = 8192;

	ReceiveBuffer(const ReceiveBuffer&) = delete;
	ReceiveBuffer& operator=(const ReceiveBuffer&) = delete;
	ReceiveBuffer(ReceiveBuffer&&) = delete;
	ReceiveBuffer& operator=(ReceiveBuffer&&) = delete;

	explicit ReceiveBuffer();
	explicit ReceiveBuffer(const int32 bufferSize);
	~ReceiveBuffer() = default;

	void MoveRear(const int32 size);

	void MoveFront(const int32 size);

	void Clear();

	[[nodiscard]]
	byte* GetReadBuffer() const;

	[[nodiscard]]
	byte* GetWriteBuffer() const;

	[[nodiscard]]
	int32 GetUseSize() const;

	[[nodiscard]]
	int32 GetFreeSize() const;

	[[nodiscard]]
	int32 GetDirectWriteSize() const;

	[[nodiscard]]
	int32 GetDirectReadSize() const;

	[[nodiscard]]
	bool IsEmpty() const;

	[[nodiscard]]
	int32 Write(const byte* pData, const int32 size);

	[[nodiscard]]
	int32 Read(byte* pBuffer, const int32 size);

	[[nodiscard]]
	int32 Peek(byte* pBuffer, const int32 size) const;

private:

	int32 mFront;
	int32 mRear;
	const int32 mBufferSize;
	std::unique_ptr<byte[], BufferDeleter> mBuffer;
};

