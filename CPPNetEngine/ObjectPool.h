#pragma once

#include <mimalloc.h>

template <typename T>
class ObjectPool
{
private:
	static constexpr int64 CHECKSUM_CODE = 0x0011223344556677;

	struct Node
	{
		T data;
		int64 checksum;
		Node* pNextNode;
	};

	struct alignas(16) AlignNode16
	{
		Node* pNode;
		int64 count;
	};

public:
	ObjectPool(const ObjectPool&) = delete;
	ObjectPool& operator=(const ObjectPool&) = delete;
	ObjectPool(ObjectPool&&) = delete;
	ObjectPool& operator=(ObjectPool&&) = delete;

	explicit ObjectPool(const bool bPlacementNew, const int32 poolingCount)
		: mbPlacementNew(bPlacementNew)
		, mTopAlineNode16({})
		, mAllocCount(0)
		, mPoolingCount(poolingCount)
	{
		for (int32 i = 0; i < mPoolingCount.load(); ++i)
		{
			Node* pNode = allocNode(!mbPlacementNew);

			pNode->checksum = CHECKSUM_CODE;
			pNode->pNextNode = mTopAlineNode16.pNode;
			mTopAlineNode16.pNode = pNode;
		}
	}

	~ObjectPool()
	{
		Node* pNode = mTopAlineNode16.pNode;

		while (pNode != nullptr)
		{
			Node* pNextNode = pNode->pNextNode;

			if (mbPlacementNew == false)
			{
				pNode->data.~T();
			}

			mi_free(pNode);

			pNode = pNextNode;

			mPoolingCount.fetch_sub(1);
		}

		if (mAllocCount.load() != 0 || mPoolingCount.load() != 0)
		{
			// TODO : 메모리 누수 로그
		}
	}

	[[nodiscard]]
	T* Alloc()
	{
		mAllocCount.fetch_add(1);
		
		AlignNode16 expected{};
		AlignNode16 desired{};

		std::atomic_ref<AlignNode16> topAlignNode16(mTopAlineNode16);

		do
		{
			expected.count = mTopAlineNode16.count;

			std::atomic_thread_fence(std::memory_order_seq_cst);

			expected.pNode = mTopAlineNode16.pNode;
			if (expected.pNode == nullptr)
			{
				Node* pNode = allocNode(true);
				pNode->checksum = CHECKSUM_CODE;
				pNode->pNextNode = nullptr;

				return &pNode->data;
			}

			desired.count = expected.count + 1;
			desired.pNode = expected.pNode->pNextNode;

		} while (topAlignNode16.compare_exchange_weak(expected, desired) == false);

		if (mbPlacementNew)
		{
			new(&expected.pNode->data)T();
		}

		mPoolingCount.fetch_sub(1);

		return &expected.pNode->data;
	}

	void Free(T* pData)
	{
		Node* pExpected;
		Node* pDesired = reinterpret_cast<Node*>(pData);
		if (pDesired->checksum != CHECKSUM_CODE)
		{
			//TODO : 로그 & 크래쉬 덤프
		}

		std::atomic_ref<Node*> topNodePtr(mTopAlineNode16.pNode);

		do
		{
			pExpected = mTopAlineNode16.pNode;

			std::atomic_thread_fence(std::memory_order_seq_cst);

			pDesired->pNextNode = pExpected;

		} while (topNodePtr.compare_exchange_weak(pExpected, pDesired) == false);

		mAllocCount.fetch_sub(1);

		mPoolingCount.fetch_add(1);
	}

	[[nodiscard]]
	int32 AllocCount() const
	{
		return mAllocCount.load();
	}

	[[nodiscard]]
	int32 PoolingCount() const
	{
		return mPoolingCount.load();
	}

private:

	Node* allocNode(const bool bPlacementNew)
	{
		Node* pNode = static_cast<Node*>(mi_malloc(sizeof(Node)));
		if (pNode == nullptr)
		{
			//TODO : 로그 & 크래시 덤프

			return nullptr;
		}

		if (bPlacementNew)
		{
			new(&pNode->data)T();
		}

		return pNode;
	}

	const bool mbPlacementNew;

	AlignNode16 mTopAlineNode16;

	std::atomic<int32> mAllocCount;

	std::atomic<int32> mPoolingCount;
};
