#pragma once
#include <Logger/Logger.h>
#include "Helpers.h"
#include "Macros.h"

#define LogLastError LOG_ERROR_D(L"Last error: {}", H::GetLastErrorAsString())

#define LOG_ASSERT(assertion, message, ...)                                                                                   \
	if (!assertion) {                                                                                                         \
		assert(assertion && " --> " message);                                                                                 \
		LOG_ERROR_D(EXPAND_1_VA_ARGS_(message, __VA_ARGS__));                                                                 \
	}


// undefine this macros in some module if you need disable verbose logging
#define LOG_DEBUG_VERBOSE(...) LOG_DEBUG_D(__VA_ARGS__)
#define LOG_ERROR_VERBOSE(...) LOG_ERROR_D(__VA_ARGS__)


#define ENABLE_CLASS_FULLNAME_LOGGING 1
#if ENABLE_CLASS_FULLNAME_LOGGING == 1




#define CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(className)                                                               \
	private:                                                                                                                  \
		std::string className##_fullClassNameA = ""#className;                                                                \
		std::wstring className##_fullClassNameW = L""#className;                                                              \
                                                                                                                              \
	public:                                                                                                                   \
        void SetFullClassName(std::wstring name) {                                                                            \
            LOG_DEBUG_D(L"Full class name = {}", name);                                                                       \
            this->className##_fullClassNameA = H::WStrToStr(name);                                                            \
            this->className##_fullClassNameW = name;                                                                          \
        }                                                                                                                     \
                                                                                                                              \
		const std::string& GetFullClassNameA() {                                                                              \
			return this->className##_fullClassNameA;                                                                          \
		}                                                                                                                     \
                                                                                                                              \
		const std::wstring& GetFullClassNameW() {                                                                             \
			return this->className##_fullClassNameW;                                                                          \
		}


#define LogDebugWithFullClassNameA(format, ...)                                                                                \
	LOG_DEBUG_D(std::string("[{}] ") + format, EXPAND_1_VA_ARGS_(this->GetFullClassNameA(), __VA_ARGS__))

#define LogErrorWithFullClassNameA(format, ...)                                                                                \
	LOG_ERROR_D(std::string("[{}] ") + format, EXPAND_1_VA_ARGS_(this->GetFullClassNameA() , __VA_ARGS__))

#define LogWarningWithFullClassNameA(format, ...)                                                                              \
	LOG_WARNING_D(std::string("[{}] ") + format, EXPAND_1_VA_ARGS_(this->GetFullClassNameA() , __VA_ARGS__))


#define LogDebugWithFullClassNameW(format, ...)                                                                                \
	LOG_DEBUG_D(std::wstring(L"[{}] ") + format, EXPAND_1_VA_ARGS_(this->GetFullClassNameW() , __VA_ARGS__))

#define LogErrorWithFullClassNameW(format, ...)                                                                                \
	LOG_ERROR_D(std::wstring(L"[{}] ") + format, EXPAND_1_VA_ARGS_(this->GetFullClassNameW() , __VA_ARGS__))

#define LogWarningWithFullClassNameW(format, ...)                                                                              \
	LOG_WARNING_D(std::wstring(L"[{}] ") + format, EXPAND_1_VA_ARGS_(this->GetFullClassNameW() , __VA_ARGS__))


#else
#define LogDebugWithFullClassName(formatStr, ...)
#define LogErrorWithFullClassName(formatStr, ...)
#define LogWarningWithFullClassName(formatStr, ...)
#define CLASS_FULLNAME_LOGGING_DECLARATION(className)
#define CLASS_FULLNAME_LOGGING_IMPLEMENTATION(className)
#define CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(className)
#endif