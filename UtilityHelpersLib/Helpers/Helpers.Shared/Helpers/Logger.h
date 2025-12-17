#pragma once
#include "common.h"
// NOTE: DISABLE_..._LOGGING macros must be defined at global level to guarantee that all files disabled this macros.
//       If you want disable some log marco for specific file do it manually by redefining this macro with empty body.
//       If you redefined some macros manually in header file - don't forget return original definitions for these macros at the end of file.

#if SPDLOG_SUPPORT
#include <Spdlog/LogHelpers.h>
// NOTE: the include order by design
#include "LoggingAssert.h"
#include "LoggingHresult.h"
#include "LoggingVerbose.h"
#include "LoggingMultifile.h"
#include "LoggingThread.h"
#include "LoggingErrors.h" // include last because inside can be includes that may use the macros above
#else
// TODO: try rewrite this workaround (mb use logger as dll)
#define LOG_CTX
#define LOG_RAW(fmt, ...)
#define LOG_TIME(fmt, ...)

#define LOG_DEBUG(fmt, ...)
#define LOG_ERROR(fmt, ...)
#define LOG_WARNING(fmt, ...)

#define LOG_DEBUG_S(_This, fmt, ...)
#define LOG_ERROR_S(_This, fmt, ...)
#define LOG_WARNING_S(_This, fmt, ...)

#define LOG_DEBUG_D(fmt, ...)
#define LOG_ERROR_D(fmt, ...)
#define LOG_WARNING_D(fmt, ...)

#define LOG_FUNCTION_ENTER_S(_This, fmt, ...)
#define LOG_FUNCTION_ENTER_C(fmt, ...)
#define LOG_FUNCTION_ENTER(fmt, ...)

#define LOG_FUNCTION_SCOPE_S(_This, fmt, ...)
#define LOG_FUNCTION_SCOPE_C(fmt, ...)
#define LOG_FUNCTION_SCOPE(fmt, ...)

#define CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(className)
#endif