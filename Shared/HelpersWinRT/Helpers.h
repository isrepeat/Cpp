#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <tchar.h>
#include <tlhelp32.h>
#include <utility>
#include <guiddef.h>
#include <KnownFolders.h>

namespace HelpersWinRT
{
    std::vector<std::string> split(std::string str, const std::string& delim);
    std::vector<std::wstring> split(std::wstring str, const std::wstring& delim);
    std::wstring CreateStringParams(const std::vector<std::pair<std::wstring, std::wstring>>& params);
    std::vector<std::wstring> ParseArgsFromString(const std::wstring& str);
    std::wstring ReplaceSubStr(std::wstring src, const std::wstring& subStr, const std::wstring& newStr);
    std::string WStrToStr(const std::wstring& wstr, int codePage = CP_ACP); // TODO: rewrite all with UTF_8 by default
    std::wstring StrToWStr(const std::string& str, int codePage = CP_ACP);
    std::wstring VecToWStr(const std::vector<wchar_t>& vec);
    std::string VecToStr(const std::vector<char>& vec);
    std::wstring ExeFullnameW();
    std::string ExeFullnameA();
    std::wstring ExePathW();
    std::string ExePathA();
}
