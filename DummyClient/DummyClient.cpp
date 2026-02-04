#include "pch.h"
#include "CrashHandler.h"
#include "MemoryAllocator.h"

int32 main()
{
    CrashHandler::Install(L"Test", L"1.0.0", L"");

    void* p = MemoryAllocator::GetInstance().Alloc(5000);

	MemoryAllocator::GetInstance().Free(p, 5000);
}
