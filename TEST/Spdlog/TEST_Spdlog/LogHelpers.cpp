#include "LogHelpers.h"
#include <Helpers/Filesystem.hpp>
#include <fstream>
#include <cassert>
#include <set>



namespace {
    //const uintmax_t maxSizeLogFile = 1 * 1024 * 1024; // 1 MB (~ 10'000 rows)
    const uintmax_t maxSizeLogFile = 10 * 1024; // 10 KB (~ 100 rows)
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
    void DefaultLoggers::Init(const std::wstring& logFilePath, bool truncate, bool appendNewSessionMsg) {
#ifdef _DEBUG
        static std::set<std::wstring> initedLoggers;
        if (initedLoggers.count(logFilePath) > 0) {
            assert(false && " ---> the logger on this path has already been initialized");
        }
        else {
            initedLoggers.insert(logFilePath);
        }
#endif

        if (!std::filesystem::exists(logFilePath)) {
            appendNewSessionMsg = false; // don't append at first created log file
        }
        else {
            auto filesize1 = H::FS::Filesize(logFilePath);
            auto filesize2 = std::filesystem::file_size(logFilePath);
            if (!truncate && std::filesystem::file_size(logFilePath) > maxSizeLogFile) {
                H::FS::RemoveBytesFromStart(logFilePath, maxSizeLogFile / 2, [] (std::ofstream& file) {
                    std::string header = "... [truncated] \n\n";
                    file.write(header.data(), header.size());
                    });
                int xxx = 9;
                //appendNewSessionMsg = false; // don't append if file recreatted again
                //truncate = true;
            }
        }

        auto& _this = GetInstance();

        _this.fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, truncate);
        _this.fileSink->set_pattern(patterns.at(Pattern::Default));
        _this.fileSink->set_level(spdlog::level::trace);

        _this.fileSinkRaw = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, truncate);
        _this.fileSinkRaw->set_pattern(patterns.at(Pattern::Raw));
        _this.fileSinkRaw->set_level(spdlog::level::trace);

        _this.fileSinkTime = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, truncate);
        _this.fileSinkTime->set_pattern(patterns.at(Pattern::Time));
        _this.fileSinkTime->set_level(spdlog::level::trace);

        _this.logger = std::make_shared<spdlog::logger>("default_logger", spdlog::sinks_init_list{ _this.fileSink });
        _this.logger->set_level(spdlog::level::trace);
        _this.logger->flush_on(spdlog::level::trace);

        _this.rawLogger = std::make_shared<spdlog::logger>("default_raw_logger", spdlog::sinks_init_list{ _this.fileSinkRaw });
        _this.rawLogger->set_level(spdlog::level::trace);
        _this.rawLogger->flush_on(spdlog::level::trace);

        _this.timeLogger = std::make_shared<spdlog::logger>("default_time_logger", spdlog::sinks_init_list{ _this.fileSinkTime });
        _this.timeLogger->set_level(spdlog::level::trace);
        _this.timeLogger->flush_on(spdlog::level::trace);

#ifdef _DEBUG
        _this.debugLogger = std::make_shared<spdlog::logger>("default_debug_logger", spdlog::sinks_init_list{ _this.debugSink, _this.fileSink });
        _this.debugLogger->set_level(spdlog::level::trace);
        _this.debugLogger->flush_on(spdlog::level::trace);
#endif

        if (appendNewSessionMsg) {
            _this.rawLogger->info("\n");
            _this.timeLogger->info("New session started");
        }
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