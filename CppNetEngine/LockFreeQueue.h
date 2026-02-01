#pragma once

#include "pch.h"
#include "ObjectPoolManager.h"

template <typename T, int32 CHUNK_SIZE = 500>
class LockFreeQueue final
{
public:

	struct Node
	{
		T data;
		Node* pNextNode;
	};

private:

	using NodeObjectPool = ObjectPoolManager<Node, CHUNK_SIZE>;

	struct AlignNode16
	{
		Node* pNode;
		int64 count;
	};

public:

	LockFreeQueue(const LockFreeQueue&) = delete;
	LockFreeQueue& operator=(const LockFreeQueue&) = delete;
	LockFreeQueue(LockFreeQueue&&) = delete;
	LockFreeQueue& operator=(LockFreeQueue&&) = delete;

	explicit LockFreeQueue(const int32 maxCount)
		: mMaxCount(maxCount)
		, mCount(0)
		, mHead{}
		, mTail{}
	{
		Node* pDummyNode = NodeObjectPool::GetInstance().Alloc();
		pDummyNode->pNextNode = nullptr;
		mHead.pNode = pDummyNode;
		mTail.pNode = pDummyNode;
	}

	~LockFreeQueue()
	{
		NodeObjectPool::GetInstance().Free(mHead.pNode);
	}

	[[nodiscard]]
	bool TryEnqueue(const T& data)
	{
		if (mMaxCount <= mCount.load())
		{
			return false;
		}

		Node* pNode = NodeObjectPool::GetInstance().Alloc();
		pNode->data = data;
		pNode->pNextNode = nullptr;

		Node* pDesired = pNode;
		std::atomic_ref<Node*> atomicTailNextNodePtr(mTail.pNode->pNextNode);

		while (true)
		{
			Node* pExpected = mTail.pNode->pNextNode;

			if (pExpected == nullptr && atomicTailNextNodePtr.compare_exchange_weak(pExpected, pDesired) == true)
			{
				moveTail();
				break;
			}
		
			moveTail();
		}

		mCount.fetch_add(1);
		
		return true;
	}

	[[nodiscard]]
	bool TryDequeue(T& outData)
	{
		if (mCount.fetch_sub(1) <= 0)
		{
			mCount.fetch_add(1);
			return false;
		}

		AlignNode16 expected{};
		AlignNode16 desired{};

		std::atomic_ref<AlignNode16> atomicHead(mHead);

		do
		{
			expected = atomicHead.load();
			if (expected.pNode == nullptr)
			{
				continue;
			}

			Node* pDequeueNode = expected.pNode->pNextNode;
			if (pDequeueNode == nullptr)
			{
				continue;
			}

			desired.count = expected.count + 1;
			desired.pNode = pDequeueNode;

			if (mHead.pNode == mTail.pNode && mTail.pNode->pNextNode != nullptr)
			{
				moveTail();
				continue;
			}

			outData = pDequeueNode->data;

		} while (atomicHead.compare_exchange_weak(expected, desired) == false);

		NodeObjectPool::GetInstance().Free(expected.pNode);

		return true;
	}

	int32 Count() const
	{
		return mCount.load();
	}

	int32 MaxCount() const noexcept
	{
		return mMaxCount;
	}

private:

	void moveTail()
	{
		AlignNode16 expected{};
		AlignNode16 desired{};

		std::atomic_ref<AlignNode16> atomicTail(mTail);

		do
		{
			expected.count = mTail.count;
			std::atomic_thread_fence(std::memory_order_seq_cst);
			expected.pNode = mTail.pNode;

			Node* pNextNode = expected.pNode->pNextNode;
			if (pNextNode == nullptr)
			{
				return;
			}

			desired.count = expected.count + 1;
			desired.pNode = pNextNode;
		} while (atomicTail.compare_exchange_weak(expected, desired) == false);
	}

	const int32 mMaxCount;

	alignas(std::hardware_destructive_interference_size) std::atomic<int32> mCount;

	alignas(std::hardware_destructive_interference_size) AlignNode16 mHead;

	alignas(std::hardware_destructive_interference_size) AlignNode16 mTail;
};
