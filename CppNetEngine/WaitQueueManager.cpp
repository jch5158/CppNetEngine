#include "pch.h"
#include "WaitQueueManager.h"

WaitQueueManager::WaitQueueManager(const int32 waitQueueSize)
	: mWaitQueueTicket{}
	, mEnterWaitQueue(waitQueueSize)
{
}

int32 WaitQueueManager::EnterWaitQueue(const SessionRef& pSession)
{
	if (mEnterWaitQueue.TryEnqueue(pSession))
	{
		const std::atomic_ref<int32> waitTicket(mWaitQueueTicket.waitTicket);
		const int32 retTicket = waitTicket.fetch_add(1);
		return retTicket;
	}

	return -1;
}

SessionRef WaitQueueManager::DequeueWaitQueue()
{
	const std::atomic_ref<int32> enterTicket(mWaitQueueTicket.enterTicket);
	while (!mEnterWaitQueue.IsEmpty())
	{
		SessionWeak pSessionWeak;
		if (mEnterWaitQueue.TryDequeue(pSessionWeak) == true)
		{
			// ReSharper disable once CppExpressionWithoutSideEffects
			enterTicket.fetch_add(1);

			SessionRef pSession = pSessionWeak.lock();
			if (pSession == nullptr)
			{
				continue;
			}

			return pSession;
		}
	}

	ticketClear();

	return nullptr;
}

int32 WaitQueueManager::GetWaitCount(const int32 myTicket) const
{
	const int32 enterTicket = mWaitQueueTicket.enterTicket;

	if (enterTicket >= myTicket)
	{
		return enterTicket - myTicket;
	}

	return 0;
}

void WaitQueueManager::ticketClear()
{
	TicketInfo expected{};
	const std::atomic_ref<TicketInfo> ticketInfo(mWaitQueueTicket);
	do
	{
		expected.waitTicket = mWaitQueueTicket.waitTicket;
		expected.enterTicket = mWaitQueueTicket.enterTicket;

	} while (!ticketInfo.compare_exchange_weak(expected, { 0,0 }));
}
