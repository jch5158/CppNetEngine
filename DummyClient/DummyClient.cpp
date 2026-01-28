#include "pch.h"
#include <cstdint>
#include <stack>

#include "LockFreeStack.h"
#include "ObjectPool.h"

using namespace std::chrono_literals;

struct alignas(16) Node
{
    long long num1;
    long long num2;
};

namespace
{

    Node gNode{};

    constexpr int32 gTestCount = 1000;

    LockFreeStack<int32*> gStack(gTestCount * 3);

    int64 gSum = 0;
}

int32 main()
{
    while (true)
    {
        {
            //생산자 스레드
            std::thread producer1([&]()->void {

                for (int32 i = 0; i < gTestCount; ++i)
                {
                    int32* pNewNum = new int32(i);
                    while (!gStack.TryPush(pNewNum))
                    {
                    }
                }
                });

            std::thread producer2([&]()->void {

                for (int32 i = 0; i < gTestCount; ++i)
                {
                    int32* pNewNum = new int32(i);
                    while (!gStack.TryPush(pNewNum))
                    {
                    }
                }
                });

            std::thread producer3([&]()->void {


                for (int32 i = 0; i < gTestCount; ++i)
                {
                    int32* pNewNum = new int32(i);
                    while (!gStack.TryPush(pNewNum))
                    {
                    }
                }
                });

            // 소비자 스레드
            std::thread consumer1([&]()->void {

                std::atomic_ref<int64> sum(gSum);

                for (int32 i = 0; i < gTestCount; ++i)
                {
                    int32* pVal = nullptr;

                    while (!gStack.TryPop(pVal))
                    {
                    }

                    sum += *pVal;
                }
                });

            std::thread consumer2([&]()->void {

                std::atomic_ref<int64> sum(gSum);

                for (int32 i = 0; i < gTestCount; ++i)
                {
                    int32* pVal = nullptr;

                    while (!gStack.TryPop(pVal))
                    {
                    }

                    sum += *pVal;
                }
                });


            std::thread consumer3([&]()->void {

                std::atomic_ref<int64> sum(gSum);

                for (int32 i = 0; i < gTestCount; ++i)
                {
                    int32* pVal = nullptr;

                    while (!gStack.TryPop(pVal))
                    {
                    }

                    sum += *pVal;
                }
                });

            producer1.join();
            producer2.join();
            producer3.join();
            consumer1.join();
            consumer2.join();
            consumer3.join();
        }

        int64 result = 0;

        for (int y = 0; y < 3; ++y)
        {
            for (int32 i = 0; i < gTestCount; ++i)
            {
                result += i;
            }
        }

        if (gSum != result)
        {
            fmt::print(L"Failed sum : {}, result : {}, alloc : {}, pooling : {}\n", gSum, result, gStack.mObjectPool.AllocCount(), gStack.mObjectPool.PoolingCount());
        }
        else
        {
            fmt::print(L"Success sum : {}, result : {}, alloc : {}, pooling : {}\n", gSum, result, gStack.mObjectPool.AllocCount(), gStack.mObjectPool.PoolingCount());
        }
        
        result = 0;
        gSum = 0;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
