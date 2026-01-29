#include "pch.h"

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


int32 main()
{
    constexpr int32 TEST_COUNT = 10000;
    LockFreeStack<Player*> playerStack(TEST_COUNT);

    //생산자 스레드
    std::thread producer1([&]()->void {

        while (true)
        {
            auto* p = new Player();
            while (!playerStack.TryPush(p))
            {
            }


            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        });

    std::thread producer2([&]()->void {


        while (true)
        {
            auto* p = new Player();
            while (!playerStack.TryPush(p))
            {
            }


            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        });

    //std::thread producer3([&]()->void {



    //    while (true)
    //    {
    //        auto* p = new Player();
    //        while (!playerStack.TryPush(p))
    //        {
    //        }


    //        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    //    }
    //    });

    // 소비자 스레드
    std::thread consumer1([&]()->void {

        while (true)
        {
            Player* p = nullptr;

            while (!playerStack.TryPop(p))
            {
            }

            net_engine_global::CrashIf(p->GetNum() != 1);

            delete p;

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        });

    std::thread consumer2([&]()->void {

        while (true)
        {
            Player* p = nullptr;

            while (!playerStack.TryPop(p))
            {
            }

            net_engine_global::CrashIf(p->GetNum() != 1);

            delete p;

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        });


    std::thread consumer3([&]()->void {

        while (true)
        {
            Player* p = nullptr;

            while (!playerStack.TryPop(p))
            {
            }

            net_engine_global::CrashIf(p->GetNum() != 1);

            delete p;

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        });


    while (true)
    {
        fmt::print(L"alloc : {}, pooling : {}\n", ObjectPoolManager<LockFreeStack<Player*>::Node, 10>::GetInstance().AllocCount(), ObjectPoolManager<LockFreeStack<Player*>::Node, 10>::GetInstance().PoolingCount());

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
