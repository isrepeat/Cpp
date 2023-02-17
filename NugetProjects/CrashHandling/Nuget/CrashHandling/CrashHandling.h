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
	API Backtrace GetBacktrace(int SkipFrames);

    // Need comile this project with /EHa
	API void RegisterVectorHandler(PVECTORED_EXCEPTION_HANDLER handler);
	API void OpenMinidumpChannel(EXCEPTION_POINTERS* pep, std::wstring packageFolder, std::wstring channelName = L"\\\\.\\pipe\\Local\\channelDumpWriter");
}