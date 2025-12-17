#pragma once
#include "common.h"
#if SPDLOG_SUPPORT
#include <Spdlog/LogHelpers.h>
#else
#define DISABLE_VERBOSE_LOGGING
#endif


#if !defined(DISABLE_VERBOSE_LOGGING)
#define LOG_DEBUG_VERBOSE(fmt, ...) LOG_DEBUG_D(fmt, ##__VA_ARGS__)
#define LOG_ERROR_VERBOSE(fmt, ...) LOG_ERROR_D(fmt, ##__VA_ARGS__)

#define LOG_DEBUG_VERBOSE_S(_This, fmt, ...) LOG_DEBUG_S(_This, fmt, ##__VA_ARGS__)
#define LOG_ERROR_VERBOSE_S(_This, fmt, ...) LOG_ERROR_S(_This, fmt, ##__VA_ARGS__)

#define LOG_FUNCTION_ENTER_VERBOSE(fmt, ...) LOG_FUNCTION_ENTER(fmt, ##__VA_ARGS__)
#define LOG_FUNCTION_SCOPE_VERBOSE(fmt, ...) LOG_FUNCTION_SCOPE(fmt, ##__VA_ARGS__)

#define LOG_FUNCTION_ENTER_VERBOSE_C(fmt, ...) LOG_FUNCTION_ENTER_C(fmt, ##__VA_ARGS__)
#define LOG_FUNCTION_SCOPE_VERBOSE_C(fmt, ...) LOG_FUNCTION_SCOPE_C(fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG_VERBOSE(fmt, ...)
#define LOG_ERROR_VERBOSE(fmt, ...)

#define LOG_DEBUG_VERBOSE_S(_This, fmt, ...)
#define LOG_ERROR_VERBOSE_S(_This, fmt, ...)

#define LOG_FUNCTION_ENTER_VERBOSE(fmt, ...)
#define LOG_FUNCTION_SCOPE_VERBOSE(fmt, ...)

#define LOG_FUNCTION_ENTER_VERBOSE_C(fmt, ...)
#define LOG_FUNCTION_ENTER_VERBOSE_C(fmt, ...)
#endif