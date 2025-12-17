#pragma once
#include "common.h"
#if SPDLOG_SUPPORT
#include <Spdlog/LogHelpers.h>
#else
#define DISABLE_ASSERT_LOGGING
#endif


#if !defined(DISABLE_ASSERT_LOGGING) && !defined(DISABLE_ERROR_LOGGING)
// NOTE 1: 
// - use comma statement "|| (..., false)" to execute next expression
// - invert result to use it like "if (LOG_ASSERT(...))"
// NOTE 2:
// - in CONSOLE apps assert failure cause abort() call (by default). To override this behaviour use _set_error_mode(_OUT_TO_MSGBOX);
#define LOG_ASSERT2(expression, message, ...)  !( \
	(bool)(expression) || (LOG_ERROR_D(L" " message L"  {{" _CRT_WIDE(#expression) L"}}", ##__VA_ARGS__), false) || (assertm(expression, message), false) \
)

#define LOG_ASSERT1(expression, ...) LOG_ASSERT2(expression, "", ##__VA_ARGS__)
#define LOG_ASSERT(...) PP_VARIADIC_FUNC(LOG_ASSERT, ##__VA_ARGS__)

#else
#define LOG_ASSERT(expression, ...) !((bool)(expression))
#endif