#include "pch.h"

#include "CrashHandler.h"
#include "ObjectPoolManager.h"
#include "LockFreeQueue.h"
#include "LockFreeStack.h"
#include "MemoryPoolManager.h"
#include "SharedPtrAllocator.h"

class Player
{
public:
    Player() = default;
    ~Player() = default;
};


int32 main()
{
    //CrashHandler::Install(L"Test", L"1.0.0", L"");

    std::shared_ptr<Player> ptr = std::allocate_shared<Player>(SharedPtrAllocator<Player>());

    fmt::print(L"Success?\n");

}
