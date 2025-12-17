#pragma once
#include "common.h"
#include "HWindows.h"
#include "Macros.h"

#include <KnownFolders.h>
#include <filesystem>
#include <tlhelp32.h>
#include <guiddef.h>
#include <algorithm>
#include <tchar.h>
#include <utility>
#include <variant>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <map>


namespace HELPERS_NS {
    std::vector<std::string> split(std::string str, const std::string& delim);
    std::vector<std::wstring> split(std::wstring str, const std::wstring& delim);

    std::wstring CreateStringParams(const std::vector<std::pair<std::wstring, std::wstring>>& params);
    std::vector<std::wstring> ParseArgsFromString(const std::wstring& str);
    std::map<std::wstring, std::wstring> ParseArgsFromStringToMap(const std::wstring& str);
    std::vector<std::pair<std::wstring, std::wstring>> ParseArgsFromStringToPair(const std::wstring& str);

    std::wstring ReplaceSubStr(std::wstring src, const std::wstring& subStr, const std::wstring& newStr);
    std::wstring WrapInQuotes(const std::wstring& str);
    std::string WrapInQuotes(const std::string& str);

    std::string WStrToStr(const std::wstring& wstr, int codePage = CP_ACP); // TODO: rewrite all with UTF_8 by default
    std::wstring StrToWStr(const std::string& str, int codePage = CP_ACP);
    std::wstring VecToWStr(const std::vector<wchar_t>& vec);
    std::string VecToStr(const std::vector<char>& vec);

    std::filesystem::path ExePath();
    std::filesystem::path ExeFullname();
#if COMPILE_FOR_DESKTOP
    std::filesystem::path GetModulePath(std::filesystem::path moduleName);
    std::filesystem::path GetKnownFolder(GUID knownFolderGUID);
    std::filesystem::path GetAppDataPath();
#endif

    std::wstring GetFormatedErrorMessage(DWORD errorMessageId);
    std::wstring GetLastErrorAsString();
#if COMPILE_FOR_DESKTOP
    std::wstring GetWSALastErrorAsString(); // return last sockets error

    int GetProcessBitDepth(std::wstring processName);
    DWORD GetProcessID(const std::wstring& processName);
    MODULEENTRY32 CheckDllInProcess(DWORD dwPID, std::wstring szDllname);

    BOOL ExecuteCommandLineW(std::wstring command, bool admin = false, DWORD showFlag = SW_HIDE, DWORD* exitCode = nullptr);
    BOOL ExecuteCommandLineA(std::string command, bool admin = false, DWORD showFlag = SW_HIDE, DWORD* exitCode = nullptr);

    void OpenLink(std::wstring link);
#endif

    template<typename Duration>
    uint64_t GetCurrentTimestamp() {
        auto currentTimestamp = std::chrono::high_resolution_clock::now();
        auto duration = currentTimestamp.time_since_epoch();

        auto durationMilisecs = std::chrono::template duration_cast<Duration>(duration);
        return durationMilisecs.count();
    }

    inline bool IsEven(int number) {
        return number % 2 == 0;
    }

    // NOTE: "T" must have a common(base) type(interface)
    template <typename... T>
    bool IsEmptyVariantOfPointers(const std::variant<std::unique_ptr<T>...>& variant) {
        bool emptyVariant = true;
        std::visit([&emptyVariant](auto&& arg) {
            emptyVariant = arg == nullptr;
            }, variant);

        return emptyVariant;
    }

    // Helper to get pointer type of variant or nullptr if variant has bad access
    template <typename T, typename... Types>
    std::unique_ptr<T>& GetVariantItem(std::variant<std::unique_ptr<Types>...>& variant) {
        static const std::unique_ptr<T> emptyPoiner = nullptr;

        try {
            return std::get<std::unique_ptr<T>>(variant);
        }
        catch (const std::bad_variant_access&) {
            return const_cast<std::unique_ptr<T>&>(emptyPoiner); // remove const
        }
    }

    // collection must be sorted from smallest to greatest
    template<class CollectionT, class LessEqualT>
    const auto& FindNearestGreatEqualIf(const CollectionT& collection, LessEqualT lessEqual) {
        auto it = std::find_if(std::begin(collection), std::end(collection), lessEqual);

        if (it == std::end(collection)) {
            // return last(greatest) item in collection
            return *(std::end(collection) - 1);
        }

        return *it;
    }

    // collection must be sorted from smallest to greatest
    template<class CollectionT, class ItemT>
    const auto& FindNearestGreatEqual(const CollectionT& collection, const ItemT& item) {
        return FindNearestGreatEqualIf(collection,
            [&item](const auto& i) {
                return item <= i;
            });
    }
}