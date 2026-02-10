#pragma once
#include "UniquePtrUtils.h"

class ReceiveBuffer
{
public:

	constexpr static int32 DEFAULT_BUFFER_SIZE = 65535
	;

	ReceiveBuffer(const ReceiveBuffer&) = delete;
	ReceiveBuffer& operator=(const ReceiveBuffer&) = delete;
	ReceiveBuffer(ReceiveBuffer&&) = delete;
	ReceiveBuffer& operator=(ReceiveBuffer&&) = delete;

	explicit ReceiveBuffer();
	explicit ReceiveBuffer(const int32 bufferSize);
	~ReceiveBuffer() = default;

	void MoveWritePos(const int32 size);

	void MoveReadPos(const int32 size);

	void Clear();

	[[nodiscard]]
	int32 GetUseSize() const;

	[[nodiscard]]
	int32 GetFreeSize() const;

	[[nodiscard]]
	char* GetReadPointer() const;

	[[nodiscard]]
	char* GetWritePointer() const;

	[[nodiscard]]
	int32 GetLinearWriteSize() const;

	[[nodiscard]]
	int32 GetLinearReadSize() const;

	[[nodiscard]]
	bool IsEmpty() const;

	int32 Write(const char* pData, const int32 size);

	int32 Read(char* pBuffer, const int32 size);

	int32 Peek(char* pBuffer, const int32 size) const;

private:

	int32 mReadPos;
	int32 mWritePos;
	const int32 mBufferSize;
	UniquePtr<char[]> mBuffer;  // NOLINT(clang-diagnostic-padded)
};

