#pragma once

template <typename T>
class LockFreeStack final
{
public:

	struct alignas(8) Node
	{
		T* pData;
		Node* pNextNode;
	};

	struct alignas(16) AlignNode16
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
		:mMaxCount(maxCount)
		, mCount(0)
		, mTopAlineNode16({})
	{
	}

	~LockFreeStack()
	{
		Node* pNode = mTopAlineNode16.pNode;

		while (pNode != nullptr)
		{
			Node* pTempNode = pNode->pNextNode;
			
			delete pNode;

			pNode = pTempNode;
		}
	}

	[[nodiscard]]
	bool TryPush(T* pData)
	{
		if (mMaxCount <= mCount.load())
		{
			return false;
		}

		Node* pExpected{};
		Node* pDesired{};

		// TODO : 메모리풀로 대체 예정
		pDesired = new Node{};
		pDesired->pData = pData;

		std::atomic_ref<Node*> topAlign8Node(mTopAlineNode16.pNode);

		do
		{
			pExpected = mTopAlineNode16.pNode;
			
			pDesired->pNextNode = pExpected;

			if (pExpected != mTopAlineNode16.pNode)
			{
				continue;
			}

		} while (topAlign8Node.compare_exchange_weak(pExpected, pDesired) == false);

		mCount.fetch_add(1);

		return true;
	}

	bool TryPop(T** outData)
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
			expected.pNode = mTopAlineNode16.pNode;
			expected.count = mTopAlineNode16.count;

			desired.pNode = expected.pNode->pNextNode;
			desired.count = expected.count + 1;

			if (expected.pNode != mTopAlineNode16.pNode || expected.count != mTopAlineNode16.count)
			{
				continue;
			}

		} while (topAlign16Node.compare_exchange_weak(expected, desired) == false);

		*outData = expected.pNode->pData;

		// TODO : 메모리풀로 대체 예정
		delete expected.pNode;

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

	std::atomic<int32> mCount;

	AlignNode16 mTopAlineNode16;
};