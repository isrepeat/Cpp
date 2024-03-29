#pragma once
#include <string>
#include <vector>
#include <utility>
#include <Windows.h>

#ifdef __MAKE_DLL__
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif

namespace StacktraceRestorer {
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

    API Backtrace BackTrace(int SkipFrames);

    // backtrace file must be saved as UTF-8
    API Backtrace BackTrace(const std::wstring& offsetsFilename);

    // Slow stack building just now
    API std::vector<StackFrame> BuildStacktrace();

    // Slow stack building from a previously quickly obtained backtrace
    API std::vector<StackFrame> BuildStacktrace(const Backtrace& backtrace);
 
    API std::vector<StackFrame> BuildStacktrace(const Backtrace& backtrace, const std::wstring& pdbFolder);
}