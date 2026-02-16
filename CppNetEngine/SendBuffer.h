#pragma once
#include "UniquePtrUtils.h"

class SendBuffer
{
public:

	constexpr static int32 DEFAULT_BUFFER_SIZE = 65535;

	SendBuffer(const SendBuffer&) = delete;
	SendBuffer& operator=(const SendBuffer&) = delete;
	SendBuffer(SendBuffer&&) = delete;
	SendBuffer& operator=(SendBuffer&&) = delete;

	explicit SendBuffer();
	explicit SendBuffer(const int32 bufferSize);
	~SendBuffer() = default;

	[[nodiscard]]
	byte* Reserve(const int32 size) const;
	void Commit(const int32 size);

	[[nodiscard]]
	int32 FreeSize() const;

	[[nodiscard]]
	int32 UseSize() const;

	[[nodiscard]]
	int32 MaxSize() const;

	[[nodiscard]]
	byte* GetBufferPtr() const;

private:

	int32 mWritePos;
	const int32 mBufferSize;
	UniquePtr<byte[]> mBuffer;  // NOLINT(clang-diagnostic-padded)
};

