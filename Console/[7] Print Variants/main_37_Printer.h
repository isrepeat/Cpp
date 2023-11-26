//#pragma once
//#include <string>
//#include <variant>
//
//using Variant_t = std::variant<int, double>;
//
//
//template <typename... VariantArgs>
//void PrintVectorImpl(const std::string& format, VariantArgs... variants);
//
//
////template <typename... Variants>
////void PrintVectorImpl(const std::string& format, Variants&&... variants) {
////    int size = sizeof ...(Variants);
////    int xx = 9;
////}
//
//
//template <typename... Args>
//void PrintVector(const std::string& format, Args... args) {
//    PrintVectorImpl(format, std::forward<Variant_t>(args)...);
//    return;
//}
//
//
////template <typename... TVariant>
////void PrintVector(const std::string& format, TVariant... Tvar) {
////    PrintVectorImpl(format, std::forward<Variant_t>(Tvar)...);
////    return;
////}