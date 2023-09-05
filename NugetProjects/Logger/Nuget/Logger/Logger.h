#pragma once
// define these macros before first include spdlog headers
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT 
#define SPDLOG_WCHAR_FILENAMES
#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <string>
#include <memory>

#define LOGGER_API // by default is empty (also used as static library)

#ifdef __MAKE_DYNAMIC_LIBRARY__
#define LOGGER_API __declspec(dllexport) // used within Logger project
#else
#ifdef LOGGER_NUGET // this macro must be defined in nuget targets
#define LOGGER_API __declspec(dllimport) // if nuget builds as dll redefine
#endif
#endif

namespace lg {
    // TODO: rewrite singleton as shared_ptr to destroy it after all others singletons
    class LOGGER_API DefaultLoggers {
    private:
        DefaultLoggers();
        static DefaultLoggers& GetInstance();
    public:
        ~DefaultLoggers() = default;
        struct UnscopedData;

        static void Init(const std::wstring& logFilePath, bool truncate = false, bool appendNewSessionMsg = true);
        static std::shared_ptr<spdlog::logger> Logger();
        static std::shared_ptr<spdlog::logger> RawLogger();
        static std::shared_ptr<spdlog::logger> TimeLogger();
        static std::shared_ptr<spdlog::logger> DebugLogger();

    private:
        std::shared_ptr<spdlog::logger> logger;
        std::shared_ptr<spdlog::logger> rawLogger;
        std::shared_ptr<spdlog::logger> timeLogger;

        std::shared_ptr<spdlog::sinks::basic_file_sink_mt> fileSink;
        std::shared_ptr<spdlog::sinks::basic_file_sink_mt> fileSinkRaw;
        std::shared_ptr<spdlog::sinks::basic_file_sink_mt> fileSinkTime;

#ifdef _DEBUG
        std::shared_ptr<spdlog::logger> debugLogger;
        const std::shared_ptr<spdlog::sinks::msvc_sink_mt> debugSink;
#endif
        std::shared_ptr<int> token = std::make_shared<int>();
    };
}


#if defined(LOG_RAW)
#error LOG_... macros already defined
#endif
#if defined(LOG_INFO) || defined(LOG_DEBUG) || defined(LOG_ERROR) || defined(LOG_WARNING)
#error LOG_... macros already defined
#endif
#if defined(LOG_INFO_D) || defined(LOG_DEBUG_D) || defined(LOG_ERROR_D) || defined(LOG_WARNING_D)
#error LOG_... macros already defined
#endif


#define LOG_RAW(...) SPDLOG_LOGGER_CALL(lg::DefaultLoggers::RawLogger(), spdlog::level::trace, __VA_ARGS__)

#define LOG_INFO(...) SPDLOG_LOGGER_CALL(lg::DefaultLoggers::Logger(), spdlog::level::info, __VA_ARGS__)
#define LOG_DEBUG(...) SPDLOG_LOGGER_CALL(lg::DefaultLoggers::Logger(), spdlog::level::debug, __VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_LOGGER_CALL(lg::DefaultLoggers::Logger(), spdlog::level::err, __VA_ARGS__)
#define LOG_WARNING(...) SPDLOG_LOGGER_CALL(lg::DefaultLoggers::Logger(), spdlog::level::warn, __VA_ARGS__)

#define LOG_INFO_D(...) SPDLOG_LOGGER_CALL(lg::DefaultLoggers::DebugLogger(), spdlog::level::info, __VA_ARGS__)
#define LOG_DEBUG_D(...) SPDLOG_LOGGER_CALL(lg::DefaultLoggers::DebugLogger(), spdlog::level::debug, __VA_ARGS__)
#define LOG_ERROR_D(...) SPDLOG_LOGGER_CALL(lg::DefaultLoggers::DebugLogger(), spdlog::level::err, __VA_ARGS__)
#define LOG_WARNING_D(...) SPDLOG_LOGGER_CALL(lg::DefaultLoggers::DebugLogger(), spdlog::level::warn, __VA_ARGS__)
