#pragma once
#include <string>
#include <functional>
#include <Windows.h>

#define CRASH_HANDLING_API // by default is empty (also used as static library)

#ifdef __MAKE_DYNAMIC_LIBRARY__
    #define CRASH_HANDLING_API __declspec(dllexport) // used within CrashHandling project
#else
    #ifdef CRASH_HANDLING_NUGET // this macro must be defined in nuget targets
        #define CRASH_HANDLING_API __declspec(dllimport) // if nuget builds as dll redefine
    #endif
#endif


namespace CrashHandling {
    enum class ExceptionType {
        StructuredException,
        UnhandledException,
    };

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
    
    struct AdditionalInfo {
        std::wstring appCenterId;
        std::wstring appVersion; // if app version passed empty it is used current package app version or 1.0.0.0 (if it is desktop)
        std::wstring appUuid = L"00000000-0000-0000-0000-000000000001"; // unique client app id
        std::wstring backtrace;
        std::wstring exceptionMsg;
        std::wstring channelName = L"\\\\.\\pipe\\Local\\channelDumpWriter";
    };

    CRASH_HANDLING_API Backtrace GetBacktrace(int SkipFrames);
    CRASH_HANDLING_API std::wstring BacktraceToString(const Backtrace& backtrace);

    // Need comile this project with /EHa (need for Release)
    CRASH_HANDLING_API void RegisterVectorHandler(PVECTORED_EXCEPTION_HANDLER handler);
    CRASH_HANDLING_API void RegisterDefaultCrashHandler(std::function<void(EXCEPTION_POINTERS*, ExceptionType)> crashCallback);
    CRASH_HANDLING_API void GenerateCrashReport(
        EXCEPTION_POINTERS* pExceptionPtrs, 
        const AdditionalInfo& additionalInfo, 
        const std::wstring& runProtocolMinidumpWriter,
        const std::vector<std::pair<std::wstring, std::wstring>>& commandArgs = {}, 
        std::function<void(const std::wstring&)> callbackToRunProtocol = nullptr);
}