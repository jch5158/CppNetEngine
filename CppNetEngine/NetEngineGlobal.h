#pragma once

class NetEngineGlobal final
{
public:
	NetEngineGlobal(const NetEngineGlobal&) = delete;
	NetEngineGlobal& operator=(const NetEngineGlobal&) = delete;
	NetEngineGlobal(NetEngineGlobal&&) = delete;
	NetEngineGlobal& operator=(NetEngineGlobal&&) = delete;

	NetEngineGlobal() = default;
	~NetEngineGlobal() = default;

	static void Initialize();

private:

	static void singletonInit();

};