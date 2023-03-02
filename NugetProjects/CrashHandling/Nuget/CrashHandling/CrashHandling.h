#pragma once
#include <string>
#include <functional>
#include <Windows.h>

#ifdef __MAKE_DLL__
#define API __declspec(dllexport)
#else
//#define API // for static
#define API __declspec(dllimport) // for dll
#endif


namespace CrashHandling {
    struct BacktraceFrame {
        std::wstring moduleName;
        std::uint32_t RVA = 0;
    };

    struct StackFrame {
        std::wstring symbolName;
        std::wstring filename;
        std::uint32_t lineNumber = 0;
        std::uint32_t RVA = 0;
    };

    using Backtrace = std::vector<std::pair<std::wstring, std::vector<BacktraceFrame>>>; // use vector pair to keep insertion order
    
    struct AppInfo {
        std::wstring packageFolder;
        std::wstring appCenterId;
        std::wstring appVersion;
        std::wstring appUuid = L"00000000-0000-0000-0000-000000000001"; // unique client app id
        std::wstring backtrace;
        std::wstring exceptionMsg;
    };

	API Backtrace GetBacktrace(int SkipFrames);
    API std::wstring BacktraceToString(const Backtrace& backtrace);

    // Need comile this project with /EHa (need for Release)
	API void RegisterVectorHandler(PVECTORED_EXCEPTION_HANDLER handler);
	API void GenerateCrashReport(EXCEPTION_POINTERS* pep, const AppInfo& appInfo, std::wstring channelName = L"\\\\.\\pipe\\Local\\channelDumpWriter");
}