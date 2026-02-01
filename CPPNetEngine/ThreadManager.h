#pragma once

#include <functional>
#include "ISingleton.h"

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

private:

	Mutex mLock;
	std::vector<std::thread> mThreads;
};

