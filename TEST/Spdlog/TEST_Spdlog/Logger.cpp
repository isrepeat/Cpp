#include "Logger.h"
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
//#include <boost/mp11.hpp>


template<typename... Ts>
struct variant_unwrapper {
    std::variant<Ts...>& var;

    template <typename T>
    operator T() {
        return std::get<T>(var);
    }
};

//template<typename T>
//T& ExpandType(Variant_t variant) {
//    if ()
//    return variant_unwrapper{}
//}


template <typename... VariantArgs>
void PrintVectorImpl(const std::string& format, VariantArgs... variantArgs) {
    int size = sizeof ...(VariantArgs);
}



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



//namespace {
//    struct erased_type : std::vector<std::any> {};
//}
//
//namespace fmt {
//    template <>
//    struct formatter<erased_type> : formatter<std::string> {
//        //template <typename ParseContext>
//        //constexpr auto parse(ParseContext& ctx) {
//        //    return ctx.begin();
//        //}
//
//        //template <typename FormatContext>
//        auto format(const erased_type& v, format_context& ctx) {
//            auto ctx_itr = ctx.out();
//            for (auto&& i : v) {
//                bool casted = false;
//                using namespace boost::mp11;
//                mp_for_each<mp_list<int, const char*, double>>([&](auto t) {
//                    if (!casted) {
//                        try {
//                            ctx_itr = format_to(ctx_itr, "{}", std::any_cast<decltype(t)>(i));
//                            casted = true;
//                            //ctx_itr++;
//                        }
//                        catch (std::bad_any_cast&) {
//                        }
//                    }
//                    });
//                if (!casted) {
//                    ctx_itr = format_to(ctx_itr, " UNKNOWN");
//                }
//            }
//            return ctx_itr;
//        }
//    };
//}

//namespace fmt {
//    template <>
//    struct formatter<erased_type> {
//        template <typename ParseContext>
//        constexpr auto parse(ParseContext& ctx) { 
//            return ctx.begin(); 
//        }
//
//        template <typename FormatContext>
//        auto format(const erased_type& v, FormatContext& ctx) {
//            auto ctx_itr = ctx.begin();
//            for (auto&& i : v) {
//                bool b = false;
//                using namespace boost::mp11;
//                mp_for_each<mp_list<int, const char*, double>>([&](auto t) {
//                    if (!b) {
//                        try {
//                            format_to(ctx_itr, " {}", std::any_cast<decltype(t)>(i));
//                            b = true;
//                            ctx_itr++;
//                        }
//                        catch (std::bad_any_cast&) {
//                        }
//                    }
//                    });
//                if (!b) {
//                    format_to(ctx_itr++, " UNKNOWN");
//                }
//            }
//            return ctx_itr;
//        }
//    };
//}



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

void Logger::log_impl_any(const std::string& format, std::vector<std::any> vec) {
    //impl->GetLogger()->info("{0}", erased_type{ std::move(vec) });
    

    //for (auto&& anyItem : vec) {
    //    bool casted = false;
    //    using namespace boost::mp11;
    //    mp_for_each<mp_list<int, const char*, double>>(
    //        [&](auto t) {
    //            if (!casted) {
    //                try {
    //                    std::cout << std::any_cast<decltype(t)>(anyItem) << " ";
    //                    casted = true;
    //                }
    //                catch (std::bad_any_cast&) {
    //                    int xxx = 9;
    //                }
    //            }
    //        });

    //    if (!casted) {
    //        std::cout << "UNKNOWN ";
    //    }
    //}
    //std::cout << std::endl;
}



//template <typename T, typename... Types>
//T& GetVariantItem(std::variant<std::unique_ptr<Types>...>& variant) {
//    static const std::unique_ptr<T> emptyPoiner = nullptr;
//
//    try {
//        return std::get<std::unique_ptr<T>>(variant);
//    }
//    catch (const std::bad_variant_access&) {
//        return const_cast<std::unique_ptr<T>&>(emptyPoiner); // remove const
//    }
//}



//void Logger::log_impl_variant(const std::string& format, std::vector<std::variant<int, const char*, double>>&& vec) {
//    for (auto&& variantItem : vec) {
//    }
//}

//template <typename... Args>
//void Logger::info(const char* fmt, Args &... args) {
//    _imp->info(fmt, args...);
//}

template <typename... VariantArgs>
void Logger::log_variant_impl(const std::string& format, VariantArgs... variants) {
    this->impl->GetLogger()->info(format, variants...);
}


template void Logger::log_variant_impl(const std::string&, Variant_t);
template void Logger::log_variant_impl(const std::string&, Variant_t, Variant_t);
template void Logger::log_variant_impl(const std::string&, Variant_t, Variant_t, Variant_t);

