#include "Helpers.h"
#include "System.h"
#include <Shlobj.h>
#include <codecvt>
#include <locale>
#include <shellapi.h>
#include <regex>


namespace H {
    namespace FS {
        void CorrectPathWithBackslashes(std::wstring& path) {
            std::replace(path.begin(), path.end(), L'/', L'\\');
        }
        void CorrectPathWithBackslashes(std::vector<std::wstring>& pathes) {
            for (auto& path : pathes) {
                std::replace(path.begin(), path.end(), L'/', L'\\');
            }
        }


        std::wstring RemoveExtFromFilenameW(std::wstring filename) {
            int n = filename.rfind(L".");
            if (n == std::wstring::npos)
                return filename;

            return filename.substr(0, n); // include last slash
        }
        std::string RemoveExtFromFilenameA(std::string filename) {
            int n = filename.rfind(".");
            if (n == std::string::npos)
                return filename;

            return filename.substr(0, n);
        }
        std::wstring GetFilenameFromPathW(std::wstring filePath) {
            int n = filePath.rfind(L"\\");
            if (n == std::wstring::npos)
                return filePath;

            return filePath.substr(n + 1);
        }
        std::string GetFilenameFromPathA(std::string filePath) {
            int n = filePath.rfind("\\");
            if (n == std::string::npos)
                return filePath;

            return filePath.substr(n + 1);
        }

        std::wstring GetPathToFileW(std::wstring filePath) {
            int n = filePath.rfind(L"\\");
            if (n == std::wstring::npos)
                return L"";

            return filePath.substr(0, n + 1); // include last slash
        }

        std::string GetPathToFileA(std::string filePath) {
            int n = filePath.rfind("\\");
            if (n == std::string::npos)
                return "";

            return filePath.substr(0, n + 1);
        }
    }

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
                result.push_back({item, L""});
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

    std::wstring GetAppDataPath() {
        wchar_t* path = nullptr;
        auto hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path);
        H::System::ThrowIfFailed(hr);
        auto result = std::wstring(path);
        CoTaskMemFree(path);

        return result;
    }


    std::wstring GetKnownFolder(GUID knownFolderGUID) {
        std::wstring result = L"";
        wchar_t* path = nullptr;
        HRESULT hr = S_OK;

        if (knownFolderGUID == FOLDERID_LocalMusic ||
            knownFolderGUID == FOLDERID_LocalVideos ||
            knownFolderGUID == FOLDERID_LocalPictures ||
            knownFolderGUID == FOLDERID_LocalDocuments ||
            knownFolderGUID == FOLDERID_LocalAppData ||
            knownFolderGUID == FOLDERID_LocalAppDataLow ||
            knownFolderGUID == FOLDERID_Profile ||
            knownFolderGUID == FOLDERID_Downloads ||
            knownFolderGUID == FOLDERID_AppsFolder ||
            knownFolderGUID == FOLDERID_Music ||
            knownFolderGUID == FOLDERID_Videos ||
            knownFolderGUID == FOLDERID_Pictures ||
            knownFolderGUID == FOLDERID_Public ||
            knownFolderGUID == FOLDERID_PublicMusic ||
            knownFolderGUID == FOLDERID_PublicVideos ||
            knownFolderGUID == FOLDERID_PublicPictures)
        {
            hr = SHGetKnownFolderPath(knownFolderGUID, 0, NULL, &path);
        }


        if (path != nullptr) {
            H::System::ThrowIfFailed(hr);
            result = std::wstring(path);
            CoTaskMemFree(path);
        }

        return result;
    }

    std::wstring GetLastErrorAsString() {
        //Get the error message ID, if any.
        DWORD errorMessageID = ::GetLastError();
        if (errorMessageID == 0) {
            return std::wstring(); //No error message has been recorded
        }

        LPWSTR messageBuffer = nullptr;

        //Ask Win32 to give us the string version of that message ID.
        //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
        size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, errorMessageID, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPWSTR)&messageBuffer, 0, NULL);
            //NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

        //Copy the error message into a std::string.
        std::wstring message(messageBuffer, size);

        //Free the Win32's string's buffer.
        LocalFree(messageBuffer);

        return message;
    }


    BOOL IsWow64(HANDLE process) {
        BOOL bIsWow64 = FALSE;

        typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
        LPFN_ISWOW64PROCESS fnIsWow64Process;
        fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

        if (NULL != fnIsWow64Process) {
            if (!fnIsWow64Process(process, &bIsWow64)) {
                //handle error
            }
        }
        return bIsWow64;
    }

    bool IsX86Process(HANDLE process) {
        SYSTEM_INFO systemInfo = { 0 };
        GetNativeSystemInfo(&systemInfo);

        // x86 environment
        if (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
            return true;

        // Check if the process is an x86 process that is running on x64 environment.
        // IsWow64 returns true if the process is an x86 process
        return IsWow64(process);
    }

    int GetProcessBitDepth(std::wstring processName) {
        int processBitDepth = 0;

        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        HANDLE hTool32 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        BOOL bProcess = Process32First(hTool32, &pe32);
        if (bProcess) {
            while ((Process32Next(hTool32, &pe32)) == TRUE) {
                if (pe32.szExeFile == processName) {
                    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
                    processBitDepth = IsX86Process(hProcess) ? 32 : 64;
                    CloseHandle(hProcess);
                    break;
                }
                //wprintf(L"%s, id = %d \n", pe32.szExeFile, pe32.th32ProcessID);
            }
        }
        CloseHandle(hTool32);
        return processBitDepth;
    }

    DWORD GetProcessID(const std::wstring& processName) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        HANDLE hTool32 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        BOOL bProcess = Process32First(hTool32, &pe32);
        if (bProcess) {
            while ((Process32Next(hTool32, &pe32)) == TRUE) {
                if (pe32.szExeFile == processName) {
                    return CloseHandle(hTool32), pe32.th32ProcessID;
                }
            }
        }
        CloseHandle(hTool32);
        return 0;
    }

    // TODO: check working in package
    MODULEENTRY32 CheckDllInProcess(DWORD dwPID, std::wstring szDllname) {
        BOOL  bMore = FALSE;
        HANDLE hSnapshot = NULL;
        MODULEENTRY32 me = { sizeof(me), };

        if ((hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwPID)) == INVALID_HANDLE_VALUE) {
            return MODULEENTRY32{ 0 };
        }

        bMore = Module32First(hSnapshot, &me);
        for (; bMore; bMore = Module32Next(hSnapshot, &me)) {
            OutputDebugStringW((L"---------- module = " + std::wstring{ me.szModule } + L" ... exePath = " + std::wstring{ me.szExePath } + L"\n").c_str());
            if (!_tcsicmp(me.szModule, szDllname.c_str()) || !_tcsicmp(me.szExePath, szDllname.c_str()))
            {
                CloseHandle(hSnapshot);
                return me;
            }
        }
        CloseHandle(hSnapshot);
        return MODULEENTRY32{ 0 };
    }

    bool ExecuteCommandLine(std::wstring parameters, bool admin, DWORD showFlag) {
        SHELLEXECUTEINFO ShExecInfo = { 0 };
        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        ShExecInfo.hwnd = NULL;
        ShExecInfo.lpVerb = admin ? L"runas" : L"open";
        ShExecInfo.lpFile = L"cmd.exe";
        ShExecInfo.lpParameters = parameters.c_str();
        ShExecInfo.lpDirectory = NULL;
        ShExecInfo.nShow = showFlag;
        ShExecInfo.hInstApp = NULL;
        bool res = ShellExecuteExW(&ShExecInfo);
        WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
        return res;
    }

    void OpenLink(std::wstring link) {
        ShellExecuteW(NULL, L"open", link.c_str(), NULL, NULL, SW_HIDE);
    }
}