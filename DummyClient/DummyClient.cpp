#include "pch.h"
#include "SendBufferAllocator.h"

int32 main()
{
	using namespace cpp_net_engine;

	CrashReporter::Init(L"DummyClient", L"1.0.0", L"");

	INetBufferRef p = MakeSendBuffer(65535);

	return 0;
}
