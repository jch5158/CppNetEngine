#pragma once

class NetEngineGlobal final
{
public:
	NetEngineGlobal(const NetEngineGlobal&) = delete;
	NetEngineGlobal& operator=(const NetEngineGlobal&) = delete;
	NetEngineGlobal(NetEngineGlobal&&) = delete;
	NetEngineGlobal& operator=(NetEngineGlobal&&) = delete;

	NetEngineGlobal() = delete;
	~NetEngineGlobal() = delete;

	static void Initialize();

private:

	static void singletonInit();
};