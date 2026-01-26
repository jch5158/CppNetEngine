#include "pch.h"
#include <cstdint>
#include <stack>

#include "LockFreeStack.h"

struct alignas(16) Node
{
    long long num1;
    long long num2;
};

namespace
{

    LockFreeStack<int32> gStack(10000);

    Node gNode{};

    constexpr int32 gTestCount = 100000;

    int64 gSum = 0;

    int32 test1[gTestCount] = { 0, };

    int32 test2[gTestCount] = { 0, };

    int32 test3[gTestCount] = { 0, };

}

int32 main()
{
     //생산자 스레드
    std::thread producer1([&]()->void {

        for (int32 i = 0; i < gTestCount; ++i)
        {
            test1[i] = i;
            while (!gStack.TryPush(&test1[i]))
            {
            }
        }
        });

    std::thread producer2([&]()->void {

        for (int32 i = 0; i < gTestCount; ++i)
        {
            test2[i] = i;
            while (!gStack.TryPush(&test2[i]))
            {
            }
        }
        });

    std::thread producer3([&]()->void {

        
        for (int32 i = 0; i < gTestCount; ++i)
        {
            test3[i] = i;
            while (!gStack.TryPush(&test3[i]))
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

            while (!gStack.TryPop(&pVal))
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

            while (!gStack.TryPop(&pVal))
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

            while (!gStack.TryPop(&pVal))
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

    int64 result = 0;
    
    for (int y = 0; y < 3; ++y)
    {
        for (int32 i = 0; i < gTestCount; ++i)
        {
            result += i;
        }
    }

    std::cout << "테스트 완료! 누적 합계: " << gSum <<  ' ' << result << '\n';

    if (gSum != result)
    {
        std::cout << "Failed\n";
    }
    else
    {
        std::cout << "Success\n";
    }
}
