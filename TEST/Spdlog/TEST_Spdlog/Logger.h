#pragma once
#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <tuple>
#include <any>
//#include <Helpers/Singleton.h>

// In order not to keep windows.h (and it deps headers) in header and avoid conflicts include only part of <winnls.h> 
#ifndef _WINNLS_
//
//  Code Page Default Values.
//  Please Use Unicode, either UTF-16 (as in WCHAR) or UTF-8 (code page CP_ACP)
//
#define CP_ACP                    0           // default to ANSI code page
#define CP_OEMCP                  1           // default to OEM  code page
#define CP_MACCP                  2           // default to MAC  code page
#define CP_THREAD_ACP             3           // current thread's ANSI code page
#define CP_SYMBOL                 42          // SYMBOL translations

#define CP_UTF7                   65000       // UTF-7 translation
#define CP_UTF8                   65001       // UTF-8 translation
#endif

namespace H {
    std::string WStrToStr(const std::wstring& wstr, int codePage = CP_ACP);
}

template<typename T>
typename std::conditional<std::is_same_v<T&&, const wchar_t*> || std::is_constructible_v<std::wstring, T&&>, std::string, T&&>::type
ApplyStringConversion(T&& t) {
    if constexpr (std::is_constructible_v<std::wstring, T&&>) {
        return H::WStrToStr(t);
    }
    else {
        return t;
    }
}


template<typename T>
class Singleton {
private:
    static Singleton<T>& GetOwnInstance() {
        static Singleton<T> instance;
        return instance;
    }

protected:
    Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    virtual ~Singleton() = default; 

public:
    template <typename... Args>
    static T& CreateInstance(Args... args) {
        auto& singleton = Singleton<T>::GetOwnInstance();
        if (singleton.instance == nullptr) {
            singleton.instance = std::make_unique<T>(args...);
        }
        return *singleton.instance;
    }

    static T& GetInstance() {
        //assert(Singleton<T>::GetOwnInstance().instance);
        return *Singleton<T>::GetOwnInstance().instance;
    }

private:
    std::unique_ptr<T> instance;
};





using Variant_t = std::variant<std::string, int, double, float>;

class Logger : public Singleton<Logger> {
    //friend class Singleton<Logger>;

public:
    Logger(std::string logFilePath);
    ~Logger();

    struct Context {
        const char* filename = nullptr;
        const char* function = nullptr;
        const int line = 0;
    };
    enum class Level {
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warning = 3,
        Error = 4,
        Critical = 5,
        Off = 6,
        N_levels
    };
    
    //static Logger& GetInstance();

    template <class... Args>
    static void log(Context context, Level level, const std::string& format, Args... args) {
        Logger::log_variant_impl(context, level, format, std::forward<Variant_t>(ApplyStringConversion(args))...);
    }

private:
    template <typename... Variants>
    static void log_variant_impl(Context context, Level level, const std::string& format, Variants... variants);

    class LoggerImpl;
    LoggerImpl* impl;
};


#define LOG_INFO(...) Logger::log(Logger::Context{__FILE__, __FUNCTION__, __LINE__}, Logger::Level::Info, __VA_ARGS__)
#define LOG_ERROR(...) Logger::log(Logger::Context{__FILE__, __FUNCTION__, __LINE__}, Logger::Level::Error, __VA_ARGS__)