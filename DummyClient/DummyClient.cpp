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

    void * p = MemoryPoolManager<>::GetInstance().Alloc(5000);

	MemoryPoolManager<>::GetInstance().Free(p, 5000);
}
