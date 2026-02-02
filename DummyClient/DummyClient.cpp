#include "pch.h"

#include "CrashHandler.h"
#include "ObjectPoolManager.h"
#include "LockFreeQueue.h"
#include "LockFreeStack.h"

class Player
{
public:
    Player()
        :mNum(0)
    {
        ++mNum;
    }

    ~Player() = default;
    
	[[nodiscard]]
    int32 GetNum() const
    {
        return mNum;
    }

    void SetNum(const int32 num)
    {
        mNum = num;
    }

private:

    int32 mNum;
};

using LockFreeQueueTest = LockFreeQueue<Player*, 1>;
using ObjectPoolMonitor = ObjectPoolManager<LockFreeQueueTest::Node, 1>;

int32 main()
{
    CrashHandler::Install(L"Test", L"1.0.0", L"");

    constexpr int32 TEST_COUNT = 10000;
    LockFreeQueueTest playerStack(TEST_COUNT);
    ObjectPool<Player> PlayerObjPool(true, 0);

    //생산자 스레드
    std::thread producer1(
        [&]()->void
        {
            while (true)
            {
                auto* p = PlayerObjPool.Alloc();
            	while (!playerStack.TryEnqueue(p))
                {
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });

    std::thread producer2(
        [&]()->void
        {
            while (true)
            {
            	auto* p = PlayerObjPool.Alloc();
            	while (!playerStack.TryEnqueue(p))
                {
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });

    std::thread producer3([&]()->void {
        while (true)
        {
            auto* p = PlayerObjPool.Alloc();
            while (!playerStack.TryEnqueue(p))
            {
            }


            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        });

    // 소비자 스레드
    std::thread consumer1(
        [&]()->void
        {
            while (true)
            {
                Player* p = nullptr;

                while (!playerStack.TryDequeue(p))
                {
                }

                CrashHandler::CrashIf(p->GetNum() != 1);

                PlayerObjPool.Free(p);

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });

    std::thread consumer2(
        [&]()->void
        {
            while (true)
            {
                Player* p = nullptr;

                while (!playerStack.TryDequeue(p))
                {
                }

                CrashHandler::CrashIf(p->GetNum() != 1);

            	PlayerObjPool.Free(p);

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });


    std::thread consumer3(
        [&]()->void
        {
            while (true)
            {
                Player* p = nullptr;

                while (!playerStack.TryDequeue(p))
                {
                }

                CrashHandler::CrashIf(p->GetNum() != 1);

            	PlayerObjPool.Free(p);

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });


    while (true)
    {
        fmt::print(L"alloc : {}, pooling : {}\n", ObjectPoolMonitor::GetInstance().AllocCount(), ObjectPoolMonitor::GetInstance().PoolingCount());

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        CrashHandler::Crash();
    }
}
