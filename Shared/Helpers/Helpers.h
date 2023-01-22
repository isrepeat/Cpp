#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <tchar.h>
#include <tlhelp32.h>
#include <utility>
#include <map>
#include <guiddef.h>
#include <KnownFolders.h>


namespace H {
    inline bool IsEven(int number) {
        return number % 2 == 0;
    }

    namespace FS {
        void CorrectPathWithBackslashes(std::wstring& path);
        void CorrectPathWithBackslashes(std::vector<std::wstring>& pathes);
        std::wstring GetFilenameFromPathW(std::wstring filePath);
        std::string GetFilenameFromPathA(std::string filePath);
    }

    std::vector<std::string> split(std::string str, const std::string& delim);
    std::vector<std::wstring> split(std::wstring str, const std::wstring& delim);
    std::wstring CreateStringParams(const std::vector<std::pair<std::wstring, std::wstring>>& params);
    std::vector<std::wstring> ParseArgsFromString(const std::wstring& str);
    std::map<std::wstring, std::wstring> ParseArgsFromStringToMap(const std::wstring& str);
    std::vector<std::pair<std::wstring, std::wstring>> ParseArgsFromStringToPair(const std::wstring& str);
    std::wstring ReplaceSubStr(std::wstring src, const std::wstring& subStr, const std::wstring& newStr);
    std::string WStrToStr(const std::wstring& wstr, int codePage = CP_ACP); // TODO: rewrite all with UTF_8 by default
    std::wstring StrToWStr(const std::string& str, int codePage = CP_ACP);
    std::wstring VecToWStr(const std::vector<wchar_t>& vec);
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

    template<typename T>
    std::vector<uint8_t> NumToBytes(T src) {
        int size = sizeof T;
        if (size == 1)
            return { (uint8_t)src };

        std::vector<uint8_t> bytes(size);
        for (int i = 0; i < size; i++) {
            bytes[i] = uint8_t(src >> i * 8) & 0xFF;
        }
        return bytes;
    }


    template<typename T>
    T BytesToNum(const std::vector<uint8_t>& bytes) {
        int size = sizeof T;
        if (bytes.size() == 0 || bytes.size() < size)
            return 0;

        if (size == 1)
            return bytes[0];

        T num = 0;
        for (int i = 0; i < size; i++) {
            num |= T(bytes[i]) << (i * 8);
        }
        return num;
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