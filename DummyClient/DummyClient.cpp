#include "pch.h"

#include "CrashHandler.h"
#include "ObjectPoolManager.h"
#include "LockFreeQueue.h"
#include "LockFreeStack.h"
#include "MemoryPoolManager.h"
#include "SharedPtrAllocator.h"


template <typename... Args>
void print(Args... tail) 
{
    ((std::cout << tail << " "), ...);
}

int32 main()
{
    CrashHandler::Install(L"Test", L"1.0.0", L"");

    // 인자의 개수나 타입이 달라도 다 받아줌
    print(1, 2.5, "hello", 'A');

}
