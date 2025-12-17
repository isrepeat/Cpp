#include "Exceptions.h"
#if COMPILE_FOR_DESKTOP
#ifdef CRASH_HANDLING_NUGET
#include <CrashHandling/CrashHandling.h>
#endif

#include "Helpers.h"
#include "Logger.h"


namespace HELPERS_NS {
    namespace System {
        ComException::ComException(HRESULT hr, const std::string& message)
            : std::exception(message.c_str())
#ifdef CRASH_HANDLING_NUGET
            , backtrace{ std::make_shared<CrashHandling::Backtrace>() }
#endif
            , errorMessage{ H::StrToWStr(message) }
            , errorCode{hr}
        {
            LOG_ERROR_D(L"Com exception = [{:#08x}] {}", static_cast<unsigned int>(hr), this->errorMessage);
        }

        ComException::ComException(HRESULT hr, const std::wstring& message)
            : std::exception(H::WStrToStr(message).c_str())
#ifdef CRASH_HANDLING_NUGET
            , backtrace{ std::make_shared<CrashHandling::Backtrace>() }
#endif
            , errorMessage{ message }
            , errorCode{ hr }
        {
            LOG_ERROR_D(L"Com exception = [{:#08x}] {}", static_cast<unsigned int>(hr), this->errorMessage);
        }

        const std::shared_ptr<CrashHandling::Backtrace>& ComException::GetBacktrace() const throw(NugetNotFoundException) {
#ifdef CRASH_HANDLING_NUGET
            return this->backtrace;
#else
            throw NugetNotFoundException();
#endif
        }

        void ComException::LogBacktrace() const throw(NugetNotFoundException) {
#ifdef CRASH_HANDLING_NUGET
            LOG_ERROR_D(L"Com exception = [{:#08x}] {}", static_cast<unsigned int>(this->errorCode), this->errorMessage);
            LOG_ERROR_D(L"\nBacktrace: \n{}", this->backtrace->GetBacktraceStr());
#else
            throw NugetNotFoundException();
#endif
        }

        std::wstring ComException::ErrorMessage() const {
            return this->errorMessage;
        }

        HRESULT ComException::ErrorCode() const {
            return this->errorCode;
        }
    }
}
#endif