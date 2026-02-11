#include "pch.h"

#include <filesystem>

#include "client/crashpad_client.h"
#include "client/crash_report_database.h"
#include "client/settings.h"

#include "CrashReporter.h"

namespace fs = std::filesystem;

// ReSharper disable All
void CrashReporter::Crash()
{
	volatile uint32* pCrash = nullptr;

#pragma warning(suppress: 6011)
	* pCrash = 0xDEAFBEFF;
}

void CrashReporter::CrashIf(const bool bCrash)
{
	if (bCrash)
	{
		Crash();
	}
}

static fs::path GetExeDirectory()
{
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    // 실행 파일의 전체 경로(예: C:\Game\Bin\Game.exe)를 가져옴
    if (GetModuleFileNameW(NULL, buffer, MAX_PATH) > 0)
    {
        fs::path exePath(buffer);
        return exePath.parent_path(); // 파일명을 제외한 디렉터리 리턴
    }
#endif

    return fs::current_path();
}

bool CrashReporter::Init(const Wstring& appName, const Wstring& appVersion, const Wstring& url)
{
    static bool sbInitialized = false;
    if (sbInitialized)
    {
        return true;
    }

    // 1. 경로 설정 (std::filesystem은 유니코드 경로를 자동 처리함)
    fs::path currentDir = GetExeDirectory();
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
    annotations["prod"] = toStdU8String(appName);
    annotations["ver"] = toStdU8String(appVersion);

    // 3. 인자 설정
    std::vector<std::string> arguments;
    arguments.emplace_back("--no-rate-limit");

    static crashpad::CrashpadClient client;

    // 5. 핸들러 시작 (Windows에서는 StartHandler가 wstring 경로를 받음)
    sbInitialized = client.StartHandler(
        base::FilePath(handlerPath.wstring()), // 경로: wstring
        base::FilePath(dbPath.wstring()),      // 경로: wstring
        base::FilePath(metricsPath.wstring()), // 경로: wstring
        toStdU8String(url),                    // URL: string (UTF-8)
        annotations,                           // 메타데이터: map<string, string>
        arguments,
        true,
        false
    );

    if (sbInitialized)
    {
        fmt::print(L"[Crashpad] Initialized. Dump Path: {}\n", dbPath.wstring());
    }
    else
    {
        fmt::print(L"[Crashpad] Initialization Failed.\n");
    }

    return sbInitialized;
}

std::string CrashReporter::toStdU8String(const Wstring& wStr)
{
	std::u8string u8Str = fs::path(wStr).u8string();

    std::string str(reinterpret_cast<const char*>(u8Str.c_str()));

	return str;
}
