#pragma once
#include <string>
#include <variant>
#include <vector>
#include <tuple>
#include <any>

using Variant_t = std::variant<int, double>;


class Logger {
public:
    Logger(std::string logFilePath);
    ~Logger();
    
    template <class... Args>
    void logVariant(const std::string& format, Args... args) {
        log_variant_impl(format, std::forward<Variant_t>(args)...);
    }

private:
    template <typename... VariantArgs>
    void log_variant_impl(const std::string& format, VariantArgs... variants);

    class LoggerImpl;
    LoggerImpl* impl;
};


#define LOG_INFO(...) clientLogger.info(__VA_ARGS__)