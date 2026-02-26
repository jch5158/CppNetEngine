#pragma once
class WaitQueueManager
{
public:

	WaitQueueManager(const WaitQueueManager&) = delete;
	WaitQueueManager operator=(const WaitQueueManager&) = delete;
	WaitQueueManager(WaitQueueManager&&) = delete;
	WaitQueueManager operator=(WaitQueueManager&&) = delete;

	explicit WaitQueueManager(const int32 waitQueueSize);
	~WaitQueueManager() = default;

	int32 EnterWaitQueue(const SessionRef& pSession);
	SessionRef DequeueWaitQueue();

	int32 GetWaitCount(const int32 myTicket) const;

private:

	struct TicketInfo
	{
		int32 waitTicket;
		int32 enterTicket;
	};

	TicketInfo mWaitQueueTicket;
	LockFreeQueue<WeakSessionRef> mEnterWaitQueue;
};

