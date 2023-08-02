#include "LogHelpers.h"
#include <filesystem>
#include <cassert>

namespace {
    const uintmax_t maxSizeLogFile = 5 * 1024 * 1024; // 5 MB
}

namespace lg {
    enum class Pattern {
        Default,
        Debug,
        Time,
        Raw,
    };

    const std::unordered_map<Pattern, std::string> patterns = {
        {Pattern::Default, "[%l] [%t] %d.%m.%Y %H:%M:%S:%e {%s:%# %!} %v"},
        {Pattern::Debug, "[dbg] [%t] {%s:%# %!} %v"},
        {Pattern::Time, "%d.%m.%Y %H:%M:%S:%e %v"},
        {Pattern::Raw, "%v"},
    };



    DefaultLoggers::DefaultLoggers()
#ifdef _DEBUG
        : debugSink{ std::make_shared<spdlog::sinks::msvc_sink_mt>() }
#endif
    {
#ifdef _DEBUG
        debugSink->set_level(spdlog::level::trace);
        debugSink->set_pattern(patterns.at(Pattern::Debug));
#endif
    }

    DefaultLoggers::~DefaultLoggers() = default;


    DefaultLoggers& DefaultLoggers::GetInstance() {
        static DefaultLoggers instance;
        return instance;
    }

    // CHECK: how it works if call multi times with same and diff logfile pathes?
    void DefaultLoggers::Init(const std::wstring& logFilePath, bool truncate) {
        assert(!spdlog::get("default_logger") && " ---> 'default_logger' already inited");
        assert(!spdlog::get("default_raw_logger") && " ---> 'default_raw_logger' already inited");
        assert(!spdlog::get("default_time_logger") && " ---> 'default_time_logger' already inited");
        assert(!spdlog::get("default_debug_logger") && " ---> 'default_debug_logger' already inited");

        if (std::filesystem::exists(logFilePath) && std::filesystem::file_size(logFilePath) > maxSizeLogFile) {
            truncate = true;
        }
        auto& _this = GetInstance();

        _this.fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, truncate);
        _this.fileSink->set_pattern(patterns.at(Pattern::Default));
        _this.fileSink->set_level(spdlog::level::trace);


        _this.logger = std::make_shared<spdlog::logger>("default_logger", spdlog::sinks_init_list{ _this.fileSink });
        _this.logger->set_level(spdlog::level::trace);
        _this.logger->flush_on(spdlog::level::trace);

        _this.rawLogger = std::make_shared<spdlog::logger>("default_raw_logger", spdlog::sinks_init_list{ _this.fileSink });
        _this.rawLogger->set_pattern(patterns.at(Pattern::Raw)); // force override fileSink pattern
        _this.rawLogger->set_level(spdlog::level::trace);
        _this.rawLogger->flush_on(spdlog::level::trace);

        _this.timeLogger = std::make_shared<spdlog::logger>("default_time_logger", spdlog::sinks_init_list{ _this.fileSink });
        _this.timeLogger->set_pattern(patterns.at(Pattern::Time)); // force override fileSink pattern
        _this.timeLogger->set_level(spdlog::level::trace);
        _this.timeLogger->flush_on(spdlog::level::trace);

#ifdef _DEBUG
        _this.debugLogger = std::make_shared<spdlog::logger>("default_debug_logger", spdlog::sinks_init_list{ _this.debugSink, _this.fileSink });
        _this.debugLogger->set_level(spdlog::level::trace);
        _this.debugLogger->flush_on(spdlog::level::trace);
#endif
        _this.rawLogger->info("\n");
        _this.timeLogger->info("New session started");
    }

    std::shared_ptr<spdlog::logger> DefaultLoggers::Logger() {
        assert(GetInstance().logger && " ---> logger is empty");
        return GetInstance().logger;
    }

    std::shared_ptr<spdlog::logger> DefaultLoggers::RawLogger() {
        assert(GetInstance().rawLogger && " ---> rawLogger is empty");
        return GetInstance().rawLogger;
    }

    std::shared_ptr<spdlog::logger> DefaultLoggers::TimeLogger() {
        assert(GetInstance().timeLogger && " ---> timeLogger is empty");
        return GetInstance().timeLogger;
    }

    std::shared_ptr<spdlog::logger> DefaultLoggers::DebugLogger() {
#ifdef _DEBUG
        assert(GetInstance().debugLogger && " ---> debugLogger is empty");
        return GetInstance().debugLogger;
#else
        assert(GetInstance().logger && " ---> logger is empty");
        return GetInstance().logger;
#endif
    }
}