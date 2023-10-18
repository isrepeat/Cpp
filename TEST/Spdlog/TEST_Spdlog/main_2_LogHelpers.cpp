#include "LogHelpers.h"
#include <spdlog/sinks/dist_sink.h>
// free falgs 'j', 'k', 'q', 'w'
#include <Helpers/Filesystem.hpp>


void TestLoggsWithDifferentPatters() {
    auto DbgSink_1 = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    DbgSink_1->set_level(spdlog::level::trace);
    DbgSink_1->set_pattern("[dbg sink 1] [%t] {%s:%# %!} %v");

    auto DbgSink_2 = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    DbgSink_2->set_level(spdlog::level::trace);
    DbgSink_2->set_pattern("[dbg sink 2] %v");

    ////auto debugProxySink_1 = std::make_shared<spdlog::sinks::dist_sink_mt>();
    ////debugProxySink_1->add_sink(sharedDbgSink);
    ////debugProxySink_1->set_level(spdlog::level::trace);
    ////debugProxySink_1->set_pattern("[Hello] [%t] {%s:%# %!} %v");

    ////auto debugProxySink_2 = std::make_shared<spdlog::sinks::dist_sink_mt>();
    ////debugProxySink_2->add_sink(sharedDbgSink);
    ////debugProxySink_2->set_level(spdlog::level::trace);
    ////debugProxySink_2->set_pattern("[Error] %v");

    ////auto formatter = std::make_unique<spdlog::pattern_formatter>();
    ////formatter->add_flag<custom_test_flag>('q', "AABB").set_pattern("[%q] [%t] {%s:%# %!} %v");
    ////dbgSink->set_formatter(std::move(formatter));

    auto debugLogger_1 = std::make_shared<spdlog::logger>("debug_logger_1", spdlog::sinks_init_list{ DbgSink_1 });
    debugLogger_1->set_level(spdlog::level::trace);
    debugLogger_1->flush_on(spdlog::level::trace);

    auto debugLogger_2 = std::make_shared<spdlog::logger>("debug_logger_2", spdlog::sinks_init_list{ DbgSink_2 });
    debugLogger_2->set_level(spdlog::level::trace);
    debugLogger_2->flush_on(spdlog::level::trace);

    //SPDLOG_LOGGER_CALL(debugLogger_1, spdlog::level::info, "Start ...");
    //SPDLOG_LOGGER_CALL(debugLogger_1, spdlog::level::info, "action = {}", 1);
    //SPDLOG_LOGGER_CALL(debugLogger_1, spdlog::level::info, "action = {}", 2);
    //SPDLOG_LOGGER_CALL(debugLogger_2, spdlog::level::info, "action = {}", 3);
    //SPDLOG_LOGGER_CALL(debugLogger_2, spdlog::level::info, "End");


    //auto sharedFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(L"logs/main-Log.txt", true);
    //sharedFileSink->set_pattern("[%l] [%t] %d.%m.%Y %H:%M:%S:%e {%s:%# %!} %v");
    //sharedFileSink->set_level(spdlog::level::trace);

    auto FileSink_1 = std::make_shared<spdlog::sinks::basic_file_sink_mt>(L"logs/main-Log.txt", true);
    FileSink_1->set_pattern("[FileSink_1] [%l] [%t] %d.%m.%Y %H:%M:%S:%e {%s:%# %!} %v");
    FileSink_1->set_level(spdlog::level::trace);

    auto FileSink_2 = std::make_shared<spdlog::sinks::basic_file_sink_mt>(L"logs/main-Log.txt", false);
    FileSink_2->set_pattern("[FileSink_2] %d.%m.%Y %H:%M:%S:%e %v");
    FileSink_2->set_level(spdlog::level::trace);


    auto fileLooger_1 = std::make_shared<spdlog::logger>("file_logger_1", spdlog::sinks_init_list{ FileSink_1 });
    fileLooger_1->set_level(spdlog::level::trace);
    fileLooger_1->flush_on(spdlog::level::trace);

    auto fileLooger_2 = std::make_shared<spdlog::logger>("file_logger_1", spdlog::sinks_init_list{ FileSink_2 });
    fileLooger_2->set_level(spdlog::level::trace);
    fileLooger_2->flush_on(spdlog::level::trace);
    
    SPDLOG_LOGGER_CALL(fileLooger_1, spdlog::level::info, "Start ...");
    SPDLOG_LOGGER_CALL(fileLooger_1, spdlog::level::info, "action = {}", 1);
    SPDLOG_LOGGER_CALL(fileLooger_1, spdlog::level::info, "action = {}", 2);
    SPDLOG_LOGGER_CALL(fileLooger_2, spdlog::level::info, "action = {}", 3);
    SPDLOG_LOGGER_CALL(fileLooger_2, spdlog::level::info, "End");
    return;
}


void TestDoubleLoggerInit() {
    lg::DefaultLoggers::Init(L"logs/main-Log.txt");
    LOG_INFO(L"Start ...");
    LOG_INFO_D("op 1 {}", false);

    lg::DefaultLoggers::Init(L"logs/main-Log.txt");

    LOG_INFO("op 2");
    LOG_WARNING_D(L"op 3 = {}", std::wstring(L"Привет"));
    LOG_ERROR("End");

    return;
}

void TestLogHelper() {
    lg::DefaultLoggers::Init(L"logs/main-Log.txt");

    auto pointer = std::make_unique<int>(0);
    pointer.reset();

    unsigned int hr = 0x80070EF5L;

    LOG_DEBUG(L"Start ...");
    LOG_INFO_D("op 1 {}", (bool)pointer);
    LOG_INFO_D("op 2 hex = {:#08x}", hr);
    LOG_WARNING_D(L"op 3 = {}", std::wstring(L"Привет"));
    LOG_RAW("------------");
    for (int i = 0; i < 100; i++) {
        LOG_ERROR("Some error = {}", i);
    }
    LOG_RAW("------------");
    LOG_DEBUG("End");
    return;
}


class custom_test_flag : public spdlog::custom_flag_formatter {
public:
    explicit custom_test_flag(std::function<std::wstring()> fn)
        : fn{ fn }
    {}

    void format(const spdlog::details::log_msg&, const std::tm&, spdlog::memory_buf_t& dest) override {
        std::wstring className = std::wstring(padinfo_.width_, ' ') + fn();
        dest.append(className.data(), className.data() + className.size());
    }

    spdlog::details::padding_info get_padding_info() {
        return padinfo_;
    }

    std::unique_ptr<custom_flag_formatter> clone() const override {
        return spdlog::details::make_unique<custom_test_flag>(fn);
    }

private:
    std::function<std::wstring()> fn;
};


template <typename T>
struct StringDeductor {
    template <typename A> StringDeductor(A) {}
    using type = T;
};

// user-defined deduction guides:
StringDeductor(const char*)->StringDeductor<char>;
StringDeductor(const wchar_t*)->StringDeductor<wchar_t>;
StringDeductor(std::string)->StringDeductor<char>;
StringDeductor(std::wstring)->StringDeductor<wchar_t>;
StringDeductor(std::string_view)->StringDeductor<char>;
StringDeductor(std::wstring_view)->StringDeductor<wchar_t>;


class CustomLogger {
public:
    CustomLogger()
        : dbgSink{ std::make_shared<spdlog::sinks::msvc_sink_mt>() }
    {
        dbgSink->set_level(spdlog::level::trace);
        dbgSink->set_pattern("[dbg sink 1] [%t] {%s:%# %!} %v");

        prefixLambda = [this] {
            return logCtx;
            };

        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        formatter->add_flag<custom_test_flag>('q', prefixLambda).set_pattern("[%t] {%s:%# %!}%q %v");
        dbgSink->set_formatter(std::move(formatter));

        debugLogger = std::make_shared<spdlog::logger>("debug_logger_1", spdlog::sinks_init_list{ dbgSink });
        debugLogger->set_level(spdlog::level::trace);
        debugLogger->flush_on(spdlog::level::trace);
    }

    std::shared_ptr<spdlog::logger> DebugLogger() {
        return debugLogger;
    }

    //// Use _impl to deduct T in std::basic_string<T>
    //template<typename TClass, typename T, typename... Args>
    //static void Log_impl(
    //    TClass* classPtr,
    //    std::shared_ptr<spdlog::logger> logger,
    //    spdlog::source_loc location, spdlog::level::level_enum level, std::basic_string<T> format, Args&&... args)
    //{
    //    if constexpr (has_member(std::remove_reference_t<decltype(std::declval<TClass>())>, __ClassFullnameLogging)) {
    //        if constexpr (std::is_same_v<T, char>) {
    //            (logger)->log(location, level, "[{}] " + format, classPtr->GetFullClassNameA(), std::forward<Args&&>(args)...);
    //        }
    //        else {
    //            (logger)->log(location, level, L"[{}] " + format, classPtr->GetFullClassNameW(), std::forward<Args&&>(args)...);
    //        }
    //        return;
    //    }
    //    (logger)->log(location, level, format, std::forward<Args&&>(args)...);
    //}

    template<typename T, typename... Args>
    void Log(std::shared_ptr<spdlog::logger> logger, spdlog::source_loc location, spdlog::level::level_enum level,
        fmt::basic_format_string<T, std::type_identity_t<Args>...> format, Args&&... args)
    {
        //using T = typename decltype(StringDeductor(format))::type;
        //Log_impl<TClass, T, Args...>(classPtr, logger, location, level, format, std::forward<Args&&>(args)...);
        logCtx = L" [Hello]";
        (logger)->log(location, level, format, std::forward<Args&&>(args)...);
    }

private:
    std::shared_ptr<spdlog::sinks::msvc_sink_mt> dbgSink;
    std::shared_ptr<spdlog::logger> debugLogger;

    std::function<std::wstring()> prefixLambda = nullptr;
    std::wstring logCtx = L"";
};

#define LOG_CTX spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}


template<typename T>
void foo_impl(std::basic_format_string<T> format) {
}

template<typename TFormat>
void foo(TFormat format) {
    foo_impl<char>(format);
}

void bar_impl(std::basic_format_string<char> fmt) {
}
void bar(const char* str) {
    //bar_impl(str);
}

void TestLogWithCustomFlags() {
    //foo_impl<char>("");
    foo("");
    //bar_impl<char>("", 1);

    const char text[] = "abcd";
    std::basic_format_string<char> bfstr1("text");
    std::basic_format_string<char> bfstr2(bfstr1);
    //std::basic_string_view<char> bsview(text);

    CustomLogger myLogger;
    myLogger.Log<char>(myLogger.DebugLogger(), LOG_CTX, spdlog::level::debug, "Start ... {}", 1234);
    //myLogger.Log(myLogger.DebugLogger(), LOG_CTX, spdlog::level::debug, "action = {}", 1);
    //myLogger.Log(myLogger.DebugLogger(), LOG_CTX, spdlog::level::debug, "action = {}", 2);

    return;
}


void main(int, char* []) {
    //TestLoggsWithDifferentPatters();
    //TestDoubleLoggerInit();
    //TestLogHelper();
    TestLogWithCustomFlags();
}
