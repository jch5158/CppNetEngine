#include "pch.h"
#include "CrashHandler.h"
#include "MemoryAllocator.h"
#include "SharedPtrAllocator.h"
#include "StlAllocator.h"
#include "LockFreeStack.h"
#include "LockFreeQueue.h"

int32 main()
{
    CrashHandler::Install(L"Test", L"1.0.0", L"");
	
	Vector<int> vec(100);

	vec.push_back(1);

	return 0;
}
