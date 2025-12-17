#pragma once
#include "common.h"
#include "HWindows.h"
#include "Macros.h"

#include <stdexcept>
#include <memory>

namespace CrashHandling {
    class Backtrace;
}

namespace HELPERS_NS {
    namespace System {
        // NOTE: For unqie_ptr over pipmpl you need define Dtor (even default) in .cpp file
#if COMPILE_FOR_DESKTOP
        class ComException : public std::exception {
        public:
            ComException(HRESULT hr, const std::string& message);
            ComException(HRESULT hr, const std::wstring& message);
            ~ComException() = default;

            ComException(const ComException&) = default;
            ComException& operator=(const ComException&) = default;

            NO_MOVE(ComException); // std::exception not support move Ctor

            // [may throw NugetNotFoundException]
            const std::shared_ptr<CrashHandling::Backtrace>& GetBacktrace() const;
            void LogBacktrace() const;

            std::wstring ErrorMessage() const;
            HRESULT ErrorCode() const;

        private:
            std::shared_ptr<CrashHandling::Backtrace> backtrace;
            std::wstring errorMessage;
            HRESULT errorCode = S_OK;
        };
#endif
    }


    class NugetNotFoundException : public std::logic_error {
    public:
        NugetNotFoundException()
            : std::logic_error("No nuget for this operation")
        {}
    };
}