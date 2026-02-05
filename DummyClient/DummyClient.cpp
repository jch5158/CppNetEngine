#include "pch.h"
#include "CrashHandler.h"
#include "MemoryAllocator.h"
#include "SharedPtrAllocator.h"
#include "StlAllocator.h"

int32 main()
{
    CrashHandler::Install(L"Test", L"1.0.0", L"");
	
	std::shared_ptr<int> ptr = std::allocate_shared<int>(SharedPtrAllocator<int>());
	return 0;
}
