#include "String.h"
#include <Windows.h>

// CP_ACP - default code page
std::string WStrToStr(const std::wstring& wstr, int codePage) {
    if (wstr.size() == 0)
        return std::string{};

    int sz = WideCharToMultiByte(codePage, 0, wstr.c_str(), -1, 0, 0, 0, 0);
    std::string res(sz, 0);
    WideCharToMultiByte(codePage, 0, wstr.c_str(), -1, &res[0], sz, 0, 0);
    return res.c_str(); // To delete '\0' use .c_str()
}