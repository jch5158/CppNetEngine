#include "pch.h"
#include "CrashHandler.h"
#include "MemoryAllocator.h"
#include "StlAllocator.h"

int32 main()
{
    CrashHandler::Install(L"Test", L"1.0.0", L"");

	Vector<int> testVector;

	testVector.resize(100);
}
