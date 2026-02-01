#pragma once

#include "pch.h"
#include "ObjectPoolManager.h"

template <typename T, int32 CHUNK_SIZE = 500>
class LockFreeStack final
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

	LockFreeStack(const LockFreeStack&) = delete;
	LockFreeStack& operator=(const LockFreeStack&) = delete;
	LockFreeStack(LockFreeStack&&) = delete;
	LockFreeStack& operator=(LockFreeStack&&) = delete;

	explicit LockFreeStack(const int32 maxCount)
		: mMaxCount(maxCount)
		, mCount(0)
		, mTopAlineNode16{}
	{
		NodeObjectPool::GetInstance();
	}

	~LockFreeStack()
	{
		Node* pNode = mTopAlineNode16.pNode;

		while (pNode != nullptr)
		{
			Node* pTempNode = pNode->pNextNode;
			
			NodeObjectPool::GetInstance().Free(pNode);

			pNode = pTempNode;
		}
	}

	[[nodiscard]]
	bool TryPush(const T& data)
	{
		if (mMaxCount <= mCount.load())
		{
			return false;
		}

		Node* pExpected{};
		Node* pDesired = NodeObjectPool::GetInstance().Alloc();

		pDesired->data = data;

		std::atomic_ref<Node*> topNodePtr(mTopAlineNode16.pNode);

		do
		{
			pExpected = mTopAlineNode16.pNode;
			pDesired->pNextNode = pExpected;

		} while (topNodePtr.compare_exchange_weak(pExpected, pDesired) == false);

		mCount.fetch_add(1);

		return true;
	}

	[[nodiscard]]
	bool TryPop(T& outData)
	{
		if (mCount.fetch_sub(1) <= 0)
		{
			mCount.fetch_add(1);
			return false;
		}
		
		AlignNode16 expected{};
		AlignNode16 desired{};

		std::atomic_ref<AlignNode16> topAlign16Node(mTopAlineNode16);

		do
		{
			expected.count = mTopAlineNode16.count;
			std::atomic_thread_fence(std::memory_order_seq_cst);
			expected.pNode = mTopAlineNode16.pNode;

			desired.count = expected.count + 1;
			desired.pNode = expected.pNode->pNextNode;

		} while (topAlign16Node.compare_exchange_weak(expected, desired) == false);

		outData = expected.pNode->data;

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

	const int32 mMaxCount;

	alignas(std::hardware_destructive_interference_size) std::atomic<int32> mCount;

	alignas(std::hardware_destructive_interference_size) AlignNode16 mTopAlineNode16;
};
