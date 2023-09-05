#pragma once
#include "HWindows.h"
#include <map>
#include <vector>
#include <string>
#include <variant>
#include <memory>
#include <chrono>
#include <algorithm>
#include <tchar.h>
#include <tlhelp32.h>
#include <utility>
#include <guiddef.h>
#include <KnownFolders.h>


namespace H {
    namespace FS {
        void CorrectPathWithBackslashes(std::wstring& path);
        void CorrectPathWithBackslashes(std::vector<std::wstring>& pathes);
        std::wstring GetFilenameFromPathW(std::wstring filePath);
        std::string GetFilenameFromPathA(std::string filePath);
        std::vector<std::wstring> GetAllLogicalDrives();
        void CopyFirstItem(const std::wstring& fromDir, const std::wstring& toDir, const std::wstring& prefix = L"");
        void CopyDirContentTo(const std::wstring& fromDir, const std::wstring& toDir);
    }

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
    std::string VecToStr(const std::vector<char>& vec);

    std::wstring ExeFullnameW();
    std::string ExeFullnameA();
    std::wstring ExePathW();
    std::string ExePathA();

    std::wstring GetAppDataPath();
    std::wstring GetKnownFolder(GUID knownFolderGUID);

    std::wstring GetLastErrorAsString();

    int GetProcessBitDepth(std::wstring processName);
    DWORD GetProcessID(const std::wstring& processName);
    MODULEENTRY32 CheckDllInProcess(DWORD dwPID, std::wstring szDllname);

    bool ExecuteCommandLine(std::wstring parameters, bool admin = false, DWORD showFlag = SW_HIDE);
    void OpenLink(std::wstring link);

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

#ifdef _DEBUG
#define MIN_BEEP_DURATION 500
#define BEEP(ton, duration) Beep(ton, duration > MIN_BEEP_DURATION ? duration : MIN_BEEP_DURATION)
#define Dbreak __debugbreak()
#else
#define BEEP(ton, ms) (void)(0)
#define Dbreak
#endif
}