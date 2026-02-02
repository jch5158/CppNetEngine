#pragma once

#include "ISingleton.h"
#include <functional>

class ThreadManager final : public ISingleton<ThreadManager>
{
public:

	friend class ISingleton<ThreadManager>;

	ThreadManager(const ThreadManager&) = delete;
	ThreadManager& operator=(const ThreadManager&) = delete;
	ThreadManager(ThreadManager&&) = delete;
	ThreadManager& operator=(ThreadManager&&) = delete;

private:

	explicit ThreadManager();

public:

	virtual ~ThreadManager() override;

	void Launch(const std::function<void()>& callback);
	void JoinWithClear();

	static void InitTls();
	static void DestroyTls();

	[[nodiscard]]
	uint32 GetThreadId() const;

private:

	Mutex mLock;
	std::vector<std::thread> mThreads;

	static thread_local uint32 sTlsThreadId;
};

