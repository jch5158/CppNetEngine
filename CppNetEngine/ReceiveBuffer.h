#pragma once

class ReceiveBuffer
{
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
	char* GetReadBuffer() const;

	[[nodiscard]]
	char* GetWriteBuffer() const;

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

	int32 Write(const char* pData, const int32 size);

	int32 Read(char* pBuffer, const int32 size);

	int32 Peek(char* pBuffer, const int32 size) const;

private:

	class BufferDeleter
	{
	public:

		BufferDeleter() = delete;
		explicit BufferDeleter(const int32 size)
			:mSize(size)
		{
		}

		void operator()(char* pBuffer) const
		{
			MemoryAllocator::GetInstance().Free(pBuffer, mSize);
		}

	private:

		const int32 mSize;
	};

	int32 mFront;
	int32 mRear;
	const int32 mBufferSize;
	std::unique_ptr<char[], BufferDeleter> mBuffer;  // NOLINT(clang-diagnostic-padded)
};

