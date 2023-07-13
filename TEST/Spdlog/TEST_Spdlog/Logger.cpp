#include "Logger.h"
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

template<typename... Ts>
struct variant_unwrapper {
    std::variant<Ts...>& var;

    template <typename T>
    operator T() {
        return std::get<T>(var);
    }
};

template<>
struct fmt::formatter<Variant_t> : formatter<std::string>
{
    auto format(Variant_t variant, format_context& ctx) const -> decltype(ctx.out()) 
    {
        return std::visit([&ctx](auto&& data) {
            return format_to(ctx.out(), "{}", data);
            }, variant);;
            
    }
};



class Logger::LoggerImpl {
public:
    LoggerImpl(std::string logFilePath)  {
        this->logger = spdlog::basic_logger_mt("logger_1", logFilePath, true);
        spdlog::set_default_logger(this->logger);
        spdlog::flush_on(spdlog::level::trace);
        spdlog::set_level(spdlog::level::trace);
        spdlog::set_pattern("[%L] [%t] %d.%m.%Y %H:%M:%S:%e {%s:%# %!} %v");
    }

    std::shared_ptr<spdlog::logger> GetLogger() {
        return this->logger;
    }

private:
    std::shared_ptr<spdlog::logger> logger;
};



Logger::Logger(std::string logFilePath)
    : impl { new Logger::LoggerImpl(logFilePath) }
{}

Logger::~Logger() {
    delete impl;
}

template <typename... VariantArgs>
void Logger::log_variant_impl(const std::string& format, VariantArgs... variants) {
    this->impl->GetLogger()->info(format, variants...);
}

template void Logger::log_variant_impl(const std::string&, Variant_t);
template void Logger::log_variant_impl(const std::string&, Variant_t, Variant_t);
template void Logger::log_variant_impl(const std::string&, Variant_t, Variant_t, Variant_t);

