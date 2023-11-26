//#include <Windows.h>
//#include <iostream>
//#include <vector>
//#include <string>
//#include <tuple>
//#include <set>
//#include <array>
//#include <functional>
//
//
//
//namespace H {
//    std::string WStrToStr(const std::wstring& wstr, int codePage = CP_UTF8); // TODO: rewrite all with UTF_8 by default
//    std::wstring StrToWStr(const std::string& str, int codePage = CP_UTF8);
//
//    // CP_ACP - default code page
//    std::string WStrToStr(const std::wstring& wstr, int codePage) {
//        if (wstr.size() == 0)
//            return std::string{};
//
//        int sz = WideCharToMultiByte(codePage, 0, wstr.c_str(), -1, 0, 0, 0, 0);
//        std::string res(sz, 0);
//        WideCharToMultiByte(codePage, 0, wstr.c_str(), -1, &res[0], sz, 0, 0);
//        //return res.c_str(); // To delete '\0' use .c_str()
//        return res;
//    }
//
//    std::wstring StrToWStr(const std::string& str, int codePage) {
//        if (str.size() == 0)
//            return std::wstring{};
//
//        int sz = MultiByteToWideChar(codePage, 0, str.c_str(), -1, 0, 0);
//        std::wstring res(sz, 0);
//        MultiByteToWideChar(codePage, 0, str.c_str(), -1, &res[0], sz);
//        //return res.c_str(); // To delete '\0' use .c_str()
//        return res;
//    }
//}
//
//
//typedef char xCHAR;
////typedef wchar_t xCHAR;
//
////template<typename T>
////T bar(T t) {
////        OutputDebugStringA("-- bar -- \n");
////
////    if constexpr (std::is_same_v<T, const char*> && std::is_same_v<xCHAR, wchar_t>) {
////        //if (std::is_same_v<xCHAR, wchar_t>) {
////            OutputDebugStringA("  used const wchar_t* \n");
////            OutputDebugStringA("  passed const char* \n");
////        //}
////            return H::StrToWStr(t).c_str();
////    }
////    else if constexpr (std::is_same_v<T, const wchar_t*> && std::is_same_v<xCHAR, char>) {
////        //if (std::is_same_v<xCHAR, char>) {
////            OutputDebugStringA("  used const char* \n");
////            OutputDebugStringA("  passed const wchar_t* \n");
////
////            return H::WStrToStr(t).c_str();
////        //}
////    }
////
////    return t;
////}
//
//
////template<typename... Args>
////void foo(const xCHAR* format, Args... args)
////{
////    OutputDebugStringA("-- foo -- \n");
////    wprintf(format, args...);
////}
////
////template<typename... Args>
////void foo2(const xCHAR* format, Args... args)
////{
////    foo(format, bar(args)...);
////}
//
//
////const char* ReturnTmpCStr() {
//std::string ReturnTmpCStr() {
//    return std::string("I_am_a_temporary_string_blablablablablablablablablablablablablabla").c_str();
//    //return H::WStrToStr(L"asdasda").c_str();
//}
//
//const wchar_t* ReturnTmpCWStr() {
//    return std::wstring(L"I_am_a_temporary_string_blablablablablablablablablablablablablabla").c_str();
//    //return H::WStrToStr(L"asdasda").c_str();
//}
//
//
////template<typename T>
////std::enable_if_t<std::is_same_v<T, const char*> || std::is_same_v<T, const wchar_t*>, const xCHAR*> ApplyStringConversion(T t) {
////    OutputDebugStringA("-- bar -- \n");
////    
////    if constexpr (std::is_same_v<T, const char*> && std::is_same_v<xCHAR, wchar_t>) {
////        OutputDebugStringA("  used const wchar_t* \n");
////        OutputDebugStringA("  passed const char* \n");
////        return H::StrToWStr(t).c_str();
////    }
////    else if constexpr (std::is_same_v<T, const wchar_t*> && std::is_same_v<xCHAR, char>) {
////        OutputDebugStringA("  used const char* \n");
////        OutputDebugStringA("  passed const wchar_t* \n");
////        return H::WStrToStr(t).c_str();
////    }
////
////    return t;
////}
//
//
////template<typename T>
////T ApplyStringConversion(T t) {
////    OutputDebugStringA("-- bar -- \n");
////
////    return t;
////}
//
//
//
//
//std::vector<std::basic_string<xCHAR>> savedTemporaryStrings;
//
//template<typename T>
//typename std::conditional<std::is_same_v<T, const char*> || std::is_same_v<T, const wchar_t*>, const xCHAR*, T>::type ApplyStringConversion(T t) {
//    OutputDebugStringA("-- bar -- \n");
//    if constexpr (std::is_same_v<T, const char*> && std::is_same_v<xCHAR, wchar_t>) {
//        OutputDebugStringA("  used const wchar_t* \n");
//        OutputDebugStringA("  passed const char* \n");
//        savedTemporaryStrings.push_back(H::StrToWStr(t));
//        return savedTemporaryStrings.back().c_str();
//    }
//    else if constexpr (std::is_same_v<T, const wchar_t*> && std::is_same_v<xCHAR, char>) {
//        OutputDebugStringA("  used const char* \n");
//        OutputDebugStringA("  passed const wchar_t* \n");
//        savedTemporaryStrings.push_back(H::WStrToStr(t));
//        return savedTemporaryStrings.back().c_str();
//    }
//    else {
//        return t; // return other non-string value as it is
//    }
//}
//
//
//template<typename... Args>
//void myPrint(const xCHAR* format, Args... args)
//{
//    int argsSize = sizeof...(args);
//    if (argsSize >= savedTemporaryStrings.capacity()) {
//        savedTemporaryStrings.reserve(argsSize + 5);
//    }
//
//    auto myTuple_orig = std::make_tuple(std::forward<Args>(args)...);
//    auto myTuple_modified = std::make_tuple(ApplyStringConversion(args)...);
//    if constexpr (std::is_same_v<xCHAR, char>)
//        printf(format, ApplyStringConversion(args)...);
//    else
//        wprintf(format, ApplyStringConversion(args)...);
//
//    savedTemporaryStrings.clear();
//}
//
//
//
////template<typename T>
////const xCHAR* ApplyStringConversion(T t) {
////    OutputDebugStringA("-- bar -- \n");
////
////    if constexpr (std::is_same_v<T, const char*> && std::is_same_v<xCHAR, wchar_t>) {
////        OutputDebugStringA("  used const wchar_t* \n");
////        OutputDebugStringA("  passed const char* \n");
////        //savedStrings.push_back(H::StrToWStr(t));
////        //savedStrings.push_back(L"AAAA\0");
////        //return savedStrings.back().c_str();
////        //return H::StrToWStr(t).c_str();
////        //gPtr = ReturnTmpCWStr();
////        //return gPtr;
////        return L"AAAA";
////    }
////    else if constexpr (std::is_same_v<T, const wchar_t*> && std::is_same_v<xCHAR, char>) {
////        OutputDebugStringA("  used const char* \n");
////        OutputDebugStringA("  passed const wchar_t* \n");
////        //return H::WStrToStr(t).c_str();
////        return L"str";
////    }
////
////
////    return L"...";
////}
//
//
//
////template<typename... Args>
////void _myPrint(const xCHAR* format, Args... args)
////{
////    OutputDebugStringA("-- foo -- \n");
////    //auto [t1, t2, t3, t4] = std::tuple<Args...>(args...);
////    
////    
////    //auto myTuple = std::tuple<Args...>(args...);
////    auto myTuple = std::make_tuple(std::forward<Args>(args)...);
////    
////    wprintf(format, args...);
////
////
////    //wprintf(format, ApplyStringConversion(args)...);
////}
////
////template<typename... Args>
////void myPrint(const xCHAR* format, Args... args)
////{
////    int argsSize = sizeof...(args);
////    if (argsSize >= savedTemporaryStrings.capacity()) {
////        savedTemporaryStrings.reserve(argsSize + 5);
////    }
////
////    auto myTuple = std::make_tuple(std::forward<Args>(args)...);
////    _myPrint(format, ApplyStringConversion(args)...);
////    //_myPrint(format, args...);
////
////    savedTemporaryStrings.clear();
////}
//
//
//
//int main() {
//    //foo2(L"Args = %d, %d, %s, %d \n", 1, 2, "Hello", 4);
//
//    //myPrint(L"Args = %d, %d, %s, %d \n", 1, 2, "Hello", 4);
//    myPrint("Args =  %d, %s, %s, %s, %s \n", 111, "Hello", L"One", L"Two", "Three");
//    //myPrint(L"Args =  %d, %d, %s, %s, %d \n", 111, 222, "AAAA", "AAAA", 555);
//
//    //printf("TmpStr = %s", ReturnTmpCStr().c_str());
//
//
//    printf("\n\n");
//    //system("pause");
//    Sleep(4000);
//    return 0;
//}