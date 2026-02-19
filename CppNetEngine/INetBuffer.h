#pragma once

#include "Types.h"

class INetBuffer
{
public:
	static constexpr int32 MAX_BUFFER_SIZE = 65536;

	INetBuffer(const INetBuffer&) = delete;
	INetBuffer& operator=(const INetBuffer&) = delete;
	INetBuffer(INetBuffer&&) = delete;
	INetBuffer& operator=(INetBuffer&&) = delete;

	INetBuffer() = default;

	virtual ~INetBuffer() = default;

	virtual void Clear() = 0;

	[[nodiscard]]
	virtual int32 GetMaxSize() const = 0;

	[[nodiscard]]
	virtual int32 GetFreeSize() const = 0;

	[[nodiscard]]
	virtual int32 GetUseSize() const = 0;

	[[nodiscard]]
	virtual byte* GetBufferPtr() = 0;

	[[nodiscard]]
	virtual byte* GetReadPtr() = 0;

	[[nodiscard]]
	virtual byte* GetWritePtr() = 0;

	virtual void MoveReadPos(const int32 size) = 0;

	virtual void MoveWritePos(const int32 size) = 0;

	[[nodiscard]]
	virtual byte* Reserve(const int32 size) = 0;

	virtual void Commit(const int32 size) = 0;
};

