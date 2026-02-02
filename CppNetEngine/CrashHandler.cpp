#include "pch.h"
#include "client/crashpad_client.h"
#include "client/crash_report_database.h"
#include "client/settings.h"
#include "CrashHandler.h"
#include <filesystem>

// ReSharper disable All
void CrashHandler::Crash()
{
	volatile uint32* pCrash = nullptr;

#pragma warning(suppress: 6011)
	* pCrash = 0xDEAFBEFF;
}

void CrashHandler::CrashIf(const bool isCrash)
{
	if (isCrash)
	{
		Crash();
	}
}

bool CrashHandler::Install(const std::wstring& appName, const std::wstring& appVersion, const std::wstring& url)
{
    namespace fs = std::filesystem;

    // 1. 경로 설정 (std::filesystem은 유니코드 경로를 자동 처리함)
    fs::path currentDir = fs::current_path();
    fs::path handlerPath = currentDir / L"crashpad_handler.exe";
    fs::path dbPath = currentDir / L"crashes";
    fs::path metricsPath = currentDir / L"metrics";

    // 핸들러 확인 (wcout 사용)
    if (!fs::exists(handlerPath))
    {
        fmt::print(L"[Error] crashpad_handler not found at: {}\n", handlerPath.wstring());
        return false;
    }

    // 2. 메타데이터 변환 (wstring -> string UTF-8)
    std::map<std::string, std::string> annotations;
    annotations["format"] = "minidump";
    annotations["prod"] = toU8String(appName);    // 한글 이름이 들어올 경우를 대비해 변환
    annotations["ver"] = toU8String(appVersion);

    // 3. 인자 설정
    std::vector<std::string> arguments;
    arguments.push_back("--no-rate-limit");

    // 4. URL 변환
    std::string uploadUrl = toU8String(url);

    static crashpad::CrashpadClient client;

    // 5. 핸들러 시작 (Windows에서는 StartHandler가 wstring 경로를 받음)
    bool success = client.StartHandler(
        base::FilePath(handlerPath.wstring()), // 경로: wstring
        base::FilePath(dbPath.wstring()),      // 경로: wstring
        base::FilePath(metricsPath.wstring()), // 경로: wstring
        uploadUrl,                             // URL: string (UTF-8)
        annotations,                           // 메타데이터: map<string, string>
        arguments,
        true,
        false
    );

    if (success)
    {
        fmt::print(L"[Crashpad] Initialized. Dump Path: {}\n", dbPath.wstring());
    }
    else
    {
        fmt::print(L"[Crashpad] Initialization Failed.\n");
    }

    return success;
}

std::string CrashHandler::toU8String(const std::wstring& wStr)
{
    namespace fs = std::filesystem;

	std::u8string u8Str = fs::path(wStr).u8string();

    std::string str(u8Str.begin(), u8Str.end());

	return str;
}
