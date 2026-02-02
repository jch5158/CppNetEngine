#pragma once

class CrashHandler final
{
public:

	CrashHandler(const CrashHandler&) = delete;
	CrashHandler& operator=(const CrashHandler&) = delete;
	CrashHandler(CrashHandler&&) = delete;
	CrashHandler& operator=(CrashHandler&&) = delete;

	static void Crash();

	static void CrashIf(const bool isCrash);

	static bool Install(const std::wstring& appName, const std::wstring& appVersion, const std::wstring& url);

private:
	
	static std::string toU8String(const std::wstring& wStr);

	CrashHandler() = default;
	~CrashHandler() = default;
};
