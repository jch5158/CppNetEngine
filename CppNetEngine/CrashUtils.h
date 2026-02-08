#pragma once

class CrashUtils final
{
public:

	CrashUtils() = delete;
	~CrashUtils() = delete;
	CrashUtils(const CrashUtils&) = delete;
	CrashUtils& operator=(const CrashUtils&) = delete;
	CrashUtils(CrashUtils&&) = delete;
	CrashUtils& operator=(CrashUtils&&) = delete;

	static void Crash();

	static void CrashIf(const bool isCrash);

	static bool Install(const std::wstring& appName, const std::wstring& appVersion, const std::wstring& url);

private:
	
	static std::string toU8String(const std::wstring& wStr);
};
