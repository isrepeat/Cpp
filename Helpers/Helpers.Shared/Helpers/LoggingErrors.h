#pragma once
#include "common.h"
#if SPDLOG_SUPPORT
#include <Spdlog/LogHelpers.h>
#include "LoggingHresult.h" // ensure that LOG_FAILED(hr) and other macros defined
#else
#define DISABLE_ERROR_LOGGING
#endif


#if !defined(DISABLE_ERROR_LOGGING)
#include "Helpers.h"
#define LogLastError LOG_ERROR_D(L"Last error: {}", HELPERS_NS::GetLastErrorAsString())
#define LogWSALastError LOG_ERROR_D(L"WSA Last error: {}", HELPERS_NS::GetWSALastErrorAsString())

// NOTE: use comma instead semicolon to be able use it in "if statement" without braces
#define LOG_THROW_STD_EXCEPTION(fmt, ...) \
		LOG_ERROR_EX(fmt, ##__VA_ARGS__),  /* LOG_ERROR_EX saved last logger message */ \
		LogLastError, \
		throw std::exception(lg::DefaultLoggers::GetLastMessage().c_str())

#include "System.h"
#define LOG_THROW_IF_FAILED(hr) \
	if (FAILED(hr)) { \
		LogLastError; \
		HELPERS_NS::System::ThrowIfFailed(hr); \
	}

#else
#define LogLastError
#define LogWSALastError
#define LOG_THROW_STD_EXCEPTION(fmt, ...) throw std::exception(fmt)
#define LOG_THROW_IF_FAILED(hr) HELPERS_NS::System::ThrowIfFailed(hr)
#endif