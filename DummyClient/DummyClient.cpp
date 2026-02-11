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


	void* pData = MemoryAllocator::GetInstance().Alloc(65535);

	MemoryAllocator::GetInstance().Free(pData, 65535);

	return 0;
}