#include "LogHelpers.h"
#include "spdlog/sinks/dist_sink.h"
// free falgs 'j', 'k', 'q', 'w'

class custom_test_flag : public spdlog::custom_flag_formatter {
public:
    explicit custom_test_flag(std::string txt)
        : some_txt{ std::move(txt) }
    {}

    void format(const spdlog::details::log_msg&, const std::tm&, spdlog::memory_buf_t& dest) override
    {
        some_txt = std::string(padinfo_.width_, ' ') + some_txt;
        dest.append(some_txt.data(), some_txt.data() + some_txt.size());
    }

    spdlog::details::padding_info get_padding_info()
    {
        return padinfo_;
    }

    std::string some_txt;

    std::unique_ptr<custom_flag_formatter> clone() const override
    {
        return spdlog::details::make_unique<custom_test_flag>(some_txt);
    }
};


void TestDebugOutput() {
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

    //lg::DefaultLoggers::Init(L"logs/main-Log.txt");
   
    //LOG_INFO(L"Start ...");
    //LOG_INFO_D("op 1");
    //LOG_INFO("op 2");
    //LOG_WARNING_D(L"op 3 = {}", std::wstring(L"Привет"));
    //LOG_ERROR("End");

    return;
}



void main(int, char* []) {
    //TestBasic();
    //TestFormatted();
    //TestWrapperLogger();
    TestDebugOutput();
}


//#include "LogHelpers.h"
//
//void TestDebugOutput() {
//    lg::DefaultLoggers::Init(L"logs/main-Log.txt");
//
//    LOG_INFO(L"Start ...");
//    LOG_INFO_D("op 1");
//    LOG_INFO("op 2");
//    LOG_WARNING_D(L"op 3 = {}", std::wstring(L"Привет"));
//    LOG_ERROR("End");
//
//    return;
//}
//
//
//
//void main(int, char* []) {
//    //TestBasic();
//    //TestFormatted();
//    //TestWrapperLogger();
//    TestDebugOutput();
//}