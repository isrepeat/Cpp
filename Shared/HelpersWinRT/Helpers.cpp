#include "Helpers.h"
#include <Shlobj.h>
#include <codecvt>
#include <locale>
#include <shellapi.h>
#include <regex>


using namespace Platform;

namespace HelpersWinRT {
    std::vector<std::string> split(std::string str, const std::string& delim) {
        std::vector<std::string> result;
        int pos = 0;
        while ((pos = str.find(delim)) != std::string::npos) {
            result.push_back(str.substr(0, pos));
            str.erase(0, pos + delim.length());
        }

        result.push_back(str);
        return result;
    }

    std::vector<std::wstring> split(std::wstring str, const std::wstring& delim) {
        std::vector<std::wstring> result;
        int pos = 0;
        while ((pos = str.find(delim)) != std::wstring::npos) {
            result.push_back(str.substr(0, pos));
            str.erase(0, pos + delim.length());
        }

        result.push_back(str);
        return result;
    }

    std::wstring CreateStringParams(const std::vector<std::pair<std::wstring, std::wstring>>& params) {
        std::wstring stringParams = L"";

        for (auto& pair : params) {
            stringParams += L" " + pair.first + L" '" + pair.second + L"'"; // wrap second(value) to quotes '' for containins spaces etc
        }

        return stringParams;
    }

    std::vector<std::wstring> ParseArgsFromString(const std::wstring& str) {
        std::wregex word_regex(L"(\'[^\']*\'|[^\\s\']+)");
        auto words_begin = std::wsregex_iterator(str.begin(), str.end(), word_regex);
        auto words_end = std::wsregex_iterator();

        std::vector<std::wstring> list;
        for (std::wsregex_iterator i = words_begin; i != words_end; ++i)
        {
            std::wsmatch match = *i;
            std::wstring match_str = match.str();

            // guard running by protocol 
            if (match_str.back() == ':') // skip params with end character == ':' if this params not wrapped in quotes
                continue;

            match_str.erase(remove(match_str.begin(), match_str.end(), '\''), match_str.end()); // remove quotes
            list.push_back(match_str);
        }

        return list;
    }

    std::map<std::wstring, std::wstring> ParseArgsFromStringToMap(const std::wstring& str) {
        std::map<std::wstring, std::wstring> result;

        int counter = 0;
        std::wstring paramName;
        for (auto& item : ParseArgsFromString(str)) {
            if (IsEven(counter++)) {
                paramName = item;
                result[paramName] = L"";
            }
            else {
                result[paramName] = item;
            }
        }

        return result;
    }

    std::vector<std::pair<std::wstring, std::wstring>> ParseArgsFromStringToPair(const std::wstring& str) {
        std::vector<std::pair<std::wstring, std::wstring>> result;

        int counter = 0;
        for (auto& item : ParseArgsFromString(str)) {
            if (IsEven(counter++)) {
                result.push_back({ item, L"" });
            }
            else {
                result.back().second = item;
            }
        }

        return result;
    }

    std::wstring ReplaceSubStr(std::wstring src, const std::wstring& subStr, const std::wstring& newStr) {
        // TODO: handle case when subStr not found in src (or subStr > src)
        return src.replace(src.find(subStr), subStr.length(), newStr);
    }


    // CP_ACP - default code page
    std::string WStrToStr(const std::wstring& wstr, int codePage) {
        if (wstr.size() == 0)
            return std::string{};

        int sz = WideCharToMultiByte(codePage, 0, wstr.c_str(), -1, 0, 0, 0, 0);
        std::string res(sz, 0);
        WideCharToMultiByte(codePage, 0, wstr.c_str(), -1, &res[0], sz, 0, 0);
        return res.c_str(); // To delete '\0' use .c_str()
    }

    std::wstring StrToWStr(const std::string& str, int codePage) {
        if (str.size() == 0)
            return std::wstring{};

        int sz = MultiByteToWideChar(codePage, 0, str.c_str(), -1, 0, 0);
        std::wstring res(sz, 0);
        MultiByteToWideChar(codePage, 0, str.c_str(), -1, &res[0], sz);
        return res.c_str(); // To delete '\0' use .c_str()
    }

    std::wstring VecToWStr(const std::vector<wchar_t>& vec) {
        return std::wstring{ vec.begin(), vec.end() }.c_str(); // .c_str() remove trailing '\0'
    }

    std::string VecToStr(const std::vector<char>& vec) {
        return std::string{ vec.begin(), vec.end() }.c_str(); // .c_str() remove trailing '\0'
    }

    std::wstring ExeFullnameW() {
        wchar_t buffer[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, buffer, MAX_PATH);
        return std::wstring{ buffer };
    }
    std::string ExeFullnameA() {
        char buffer[MAX_PATH] = { 0 };
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        return std::string{ buffer };
    }

    std::wstring ExePathW() {
        wchar_t buffer[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, buffer, MAX_PATH);
        auto pos = std::wstring{ buffer }.find_last_of(L"\\/");
        return std::wstring{ buffer }.substr(0, pos);
    }
    std::string ExePathA() {
        char buffer[MAX_PATH] = { 0 };
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        auto pos = std::string{ buffer }.find_last_of("\\/");
        return std::string{ buffer }.substr(0, pos);
    }
}
