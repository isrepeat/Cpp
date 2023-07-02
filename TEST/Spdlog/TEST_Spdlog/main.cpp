//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// spdlog usage example

#include <Windows.h>
#include <cstdio>
#include <chrono>

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
//#include "spdlog/cfg/env.h"  // support for loading levels from the environment variable
//#include "spdlog/fmt/ostr.h" // support for user defined types

//struct Temp {
//    Temp() {
//        auto logger = spdlog::default_logger();
//        SPDLOG_LOGGER_CALL(logger, spdlog::level::debug, "my logger debug msg {:d}", 1488);
//    }
//    ~Temp() = default;
//};


void foo() {
    auto logger = spdlog::default_logger();
    //SPDLOG_LOGGER_CALL(logger, spdlog::level::debug, "my logger debug msg {:d}", 1488);
    spdlog::set_pattern("[%L] [%t] %d.%m.%Y %H:%M:%S:%e {%s:%# %!} %v");
    SPDLOG_LOGGER_CALL(logger, spdlog::level::debug, "my logger debug msg = {:d}", 1);
    SPDLOG_LOGGER_CALL(logger, spdlog::level::info, "my logger debug msg = {:d}", 2);
    //spdlog::set_pattern("%+");
    logger->log(spdlog::source_loc{ __FILE__, __LINE__, __FUNCTION__ }, spdlog::level::warn, "my logger debug msg 3");
    logger->log(spdlog::source_loc{ __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::err, "my logger debug msg 4");
}

int main(int, char* [])
{
    try {
        //// pass the spdlog::file_event_handlers to file sinks for open/close log file notifications
        //spdlog::file_event_handlers handlers;
        //handlers.before_open = [](spdlog::filename_t filename) { spdlog::info("Before opening {}", filename); };
        //handlers.after_open = [](spdlog::filename_t filename, std::FILE* fstream) {
        //    spdlog::info("After opening {}", filename);
        //    fputs("After opening\n", fstream);
        //};
        //handlers.before_close = [](spdlog::filename_t filename, std::FILE* fstream) {
        //    spdlog::info("Before closing {}", filename);
        //    fputs("Before closing\n", fstream);
        //};
        //handlers.after_close = [](spdlog::filename_t filename) { spdlog::info("After closing {}", filename); };
        //auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/events-sample.txt", true, handlers);
        //spdlog::logger my_logger("some_logger", file_sink);
        //my_logger.info("Some log line");

        spdlog::info("Welcome to spdlog version {}.{}.{}  !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);

        spdlog::warn("Easy padding in numbers like {:08d}", 12);
        spdlog::critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
        spdlog::info("Support for floats {:03.2f}", 1.23456);
        spdlog::info("Positional args are {1} {0}..", "too", "supported");
        spdlog::info("{:>8} aligned, {:<8} aligned", "right", "left");

        // Runtime log levels
        spdlog::set_level(spdlog::level::info); // Set global log level to info
        spdlog::debug("This message should not be displayed!");
        spdlog::set_level(spdlog::level::trace); // Set specific logger's log level
        spdlog::debug("This message should be displayed..");

        // Customize msg format for all loggers
        spdlog::set_pattern("[%H:%M:%S %z] [%^%L%$] [thread %t] %v");
        spdlog::info("This an info message with custom format");
        spdlog::set_pattern("%+"); // back to default format
        spdlog::set_level(spdlog::level::info);



        auto my_logger = spdlog::basic_logger_mt("logger_1", "logs/basic-log-1.txt", true);

        //auto cb_logger = spdlog::callback_logger_mt("custom_callback_logger", [](const spdlog::details::log_msg& /*msg*/) {
        //    // do what you need to do with msg
        //    });


        auto old_logger = spdlog::default_logger();

        spdlog::set_default_logger(my_logger);
        spdlog::set_level(spdlog::level::trace);
        spdlog::trace("Trace msg");
        spdlog::debug("Debug msg");
        spdlog::info("Info msg");
        //Temp();
        foo();
        spdlog::warn("Warning msg");
        spdlog::error("Error msg");
        spdlog::critical("Critical msg");

        spdlog::set_default_logger(old_logger);


        //// Backtrace support
        //// Loggers can store in a ring buffer all messages (including debug/trace) for later inspection.
        //// When needed, call dump_backtrace() to see what happened:
        //spdlog::enable_backtrace(10); // create ring buffer with capacity of 10  messages
        //for (int i = 0; i < 100; i++)
        //{
        //    spdlog::debug("Backtrace message {}", i); // not logged..
        //}
        //// e.g. if some error happened:
        //spdlog::dump_backtrace(); // log them now!




 




        //auto formatter = spdlog::details::make_unique<spdlog::pattern_formatter>();
        //formatter->add_flag<my_formatter_flag>('*').set_pattern("[%n] [%*] [%^%l%$] %v");
        //// set the new formatter using spdlog::set_formatter(formatter) or logger->set_formatter(formatter)
        //// spdlog::set_formatter(std::move(formatter));


        // Flush all *registered* loggers using a worker thread every 3 seconds.
        // note: registered loggers *must* be thread safe for this to work correctly!
        spdlog::flush_every(std::chrono::seconds(3));

        // Apply some function on all registered loggers
        spdlog::apply_all([&](std::shared_ptr<spdlog::logger> l) { l->info("End of example."); });

        // Release all spdlog resources, and drop all loggers in the registry.
        // This is optional (only mandatory if using windows + async log).
        spdlog::shutdown();

    }
    // Exceptions will only be thrown upon failed logger or sink construction (not during logging).
    catch (const spdlog::spdlog_ex& ex)
    {
        std::printf("Log initialization failed: %s\n", ex.what());
        return 1;
    }

    int xxx = 9;
}