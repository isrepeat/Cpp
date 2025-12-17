#pragma once
#include "libhelpers\config.h"
#include "..\Macros.h"
#include "..\Filesystem.h"
#include "..\raw_ptr.h"

#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <atomic>
#include <mutex>

namespace logging {
    class Logger {
    public:
        static void InitializeLogger(std::wstring path, raw_ptr<Filesystem::IFilesystem> filesystem);
        static void ReportMessage(std::wstring msg);
        static void ReportMessage(std::string msg);
        static void SetLoggingEnabled(bool flag);
        static bool GetIsLoggingEnabled();

        class Helper {
        public:
            static constexpr const wchar_t* GetFileName(const wchar_t* path)
            {
                const wchar_t* file = path;
                while (*path) {
                    auto chr = *path++;
                    if (chr == '\\' || chr == '/') {
                        file = path;
                    }
                }
                return file;
            }
        };

    private:
        class NativeLogger {
        public:
            NativeLogger(std::wstring path, raw_ptr<Filesystem::IFilesystem> filesystem);

            void ReportMessage(std::wstring msg);
            void ReportMessage(std::string msg);

            template<typename ... Args>
            void ReportMessage(const std::wstring& format, Args ... args) {
                int size_s = std::swprintf(nullptr, 0, format.c_str(), args ...) + 1;
                if (size_s <= 0)
                    return;

                auto size = static_cast<size_t>(size_s);
                std::unique_ptr<wchar_t[]> buf(new wchar_t[size]);
                std::swprintf(buf.get(), size, format.c_str(), args ...);
                ReportMessage(std::wstring{buf.get(), buf.get() + size - 1});
            }


            void SetIfLoggingIsEnabled(bool flag);
            bool GetIfLoggingIsEnabled();

        private:
            std::mutex fileMtx;
            std::unique_ptr<Filesystem::IFolder> folder;
            std::unique_ptr<Filesystem::IFile> file;
            std::unique_ptr<Filesystem::IStream> stream;

            std::atomic<bool> isLoggingEnabled = true;
        };

        class Singleton {
        public:
            static Singleton& Instance();

            std::mutex internalLoggerMtx;
            std::unique_ptr<NativeLogger> internalLogger;
        };

    public:
        template<typename ... Args>
        static void ReportMessage(const std::wstring& format, Args ... args) {
            auto& instance = Singleton::Instance();
            auto lk = std::lock_guard(instance.internalLoggerMtx);

            if (!instance.internalLogger) {
                return;
            }

            instance.internalLogger->ReportMessage(format, std::forward<Args>(args)...);
        }
    };
}

#define FUNCSIG1 __FUNCSIG__
#define WFUNCSIG WSTR2(FUNCSIG1)

#define LOG_FN                                                                                          \
auto __LogFn_ThId = []() {                                                                              \
    auto myid = std::this_thread::get_id();                                                             \
    std::stringstream ss;                                                                               \
    ss << myid;                                                                                         \
    std::string mystring = ss.str();                                                                    \
    return mystring;                                                                                    \
};                                                                                                      \
struct __LogFn_Scope {                                                                                  \
    std::string msg;                                                                                    \
    __LogFn_Scope(std::string msg) : msg(msg) {}                                                        \
    ~__LogFn_Scope() {                                                                                  \
        logging::Logger::ReportMessage(this->msg);                                                      \
    }                                                                                                   \
};                                                                                                      \
                                                                                                        \
__LogFn_Scope __logFnScope(__LogFn_ThId() + "<<---" + __FUNCSIG__);                                     \
logging::Logger::ReportMessage(__LogFn_ThId() + "--->>" + __FUNCSIG__);

#define DEBUG_JOIN(...) __VA_ARGS__
#define WFILE WSTR2(__FILE__)
#define LOG_REPORT(...) logging::Logger::ReportMessage(std::wstring(logging::Logger::Helper::GetFileName(WFILE)) + L":" + std::to_wstring(__LINE__) + std::wstring{L" : "} + DEBUG_JOIN(__VA_ARGS__))