#include "pch.h"
#include "ReceiveBuffer.h"
#include <process.h>
#include <Windows.h>
#include <ctime>
#include <conio.h>
#include "CrashReporter.h"
#include "TlsObjectPool.h"
#include "MemoryPool.h"

int32 main()
{
	CrashReporter::Init(L"DummyClient", L"1.0.0", L"");

	UniquePtr<int> p = UniquePtrUtils<int>::Alloc(10);

	std::cout << *p;

	return 0;
}