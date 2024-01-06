#include "main_37_Printer.h"
//
//template <typename... Args>
//void PrintFormat(const char* format, Args... args) {
//
//}

//template<typename T>
//T& ExpandType(Variant_t variant) {
//    if ()
//    return variant_unwrapper{}
//}



template<typename... Ts>
struct variant_unwrapper {
    std::variant<Ts...>& var;

    template <typename T>
    operator T() {
        return std::get<T>(var);
    }
};

template <typename... VariantArgs>
void PrintVectorImpl(const std::string& format, VariantArgs... variantArgs) {
    int size = sizeof ...(VariantArgs);
    //printf(format.c_str(), ExpandType(variantArgs)...)
    int xx = 9;
}


template void PrintVectorImpl(const std::string&, Variant_t);
template void PrintVectorImpl(const std::string&, Variant_t, Variant_t);
template void PrintVectorImpl(const std::string&, Variant_t, Variant_t, Variant_t);
