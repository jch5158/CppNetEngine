#include "pch.h"
#include "ReceiveBuffer.h"
#include <process.h>
#include <Windows.h>
#include <ctime>
#include <conio.h>
#include "CrashReporter.h"
#include "MemoryPool.h"

int32 main()
{
	CrashReporter::Init(L"DummyClient", L"1.0.0", L"");

	void* p = MemoryAllocator::GetInstance().Alloc(4096);

	*static_cast<int*>(p) = 30;

	return 0;
}