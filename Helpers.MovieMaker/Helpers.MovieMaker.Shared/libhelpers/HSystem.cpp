#include "pch.h"
#include "HSystem.h"
#include "HData.h"
#include "HText.h"
#include "Logger\NativeLogger.h"

namespace H {
    void System::DebuggerBreak(wchar_t* str) {
        if (str) {
            logging::Logger::ReportMessage(str);
        }
#if defined(HAVE_VISUAL_STUDIO) && defined(_DEBUG)
        Dbreak;
#else
        // Just place breakpoint on this line:
        int stop = 234;
#endif
    }

    void System::Assert(bool expression, wchar_t* str) {
        if (!expression) {
            System::DebuggerBreak();

            if (str) {
                logging::Logger::ReportMessage(str);
            }
        }
        assert(expression);
    }

    void System::DebugOutput(const std::string &v) {
#ifdef _DEBUG
#ifdef HAVE_VISUAL_STUDIO
        OutputDebugStringA("\n");
        OutputDebugStringA(v.c_str());
#else
        printf("\n%s", v.c_str());
#endif
#endif
    }

    void System::DebugOutput(const std::wstring &v) {
#ifdef _DEBUG
#ifdef HAVE_VISUAL_STUDIO
        OutputDebugStringW(L"\n");
        OutputDebugStringW(v.c_str());
#endif
        wprintf(L"\n%s", v.c_str());
#endif
    }

    std::wstring System::FixPathWinRT(const std::wstring &v) {
        std::wstring fixed;

        if (!v.empty()) {
            size_t startIdx = 0;
            size_t size = v.size();

            if (v[startIdx] == '\\' || v[startIdx] == '/') {
                startIdx++;
            }

            if (v[size - 1] == '\\' || v[size - 1] == '/') {
                size--;
            }

            fixed.reserve(size);

            for (size_t i = startIdx; i < size; i++) {
                if (v[i] != '/') {
                    fixed.push_back(v[i]);
                }
                else {
                    fixed.push_back('\\');
                }
            }
        }

        return fixed;
    }

    void System::ThrowIfFailed(HRESULT hr, wchar_t* str) {
#if HAVE_WINRT == 1
        if (FAILED(hr)) {
            Dbreak;
            auto exception = ref new Platform::Exception(hr);
            if (str) {
                logging::Logger::ReportMessage(str);
            } else {
                logging::Logger::ReportMessage(
                    L"(" + H::Text::StringFormat(L"%X", exception->HResult) + L") "
                    + std::wstring{exception->Message->Data()});
            }
            throw exception;
        }
#else
        if (FAILED(hr)) {
            Dbreak;
            throw HResultException(hr);
        }
#endif
    }

    std::vector<uint8_t> System::LoadPackageFile(const std::string &path) {
        return System::LoadPackageFile(H::Data::Convert<std::wstring>(path));
    }

    std::vector<uint8_t> System::LoadPackageFile(const std::wstring &path) {
        std::vector<uint8_t> fileData;
        std::wstring fullPath = System::GetPackagePath() + path;
#if HAVE_WINRT == 1
        auto file = CreateFile2(fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
#else
        auto file = CreateFileW(fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif
        BOOL res = false;

        if (file != INVALID_HANDLE_VALUE) {
            DWORD readed;
            LARGE_INTEGER pos, newPos, fileSize;

            pos.QuadPart = 0;
            newPos.QuadPart = 0;

            SetFilePointerEx(file, pos, &newPos, FILE_END);
            fileSize = newPos;
            SetFilePointerEx(file, pos, &newPos, FILE_BEGIN);

            fileData.resize(static_cast<size_t>(fileSize.QuadPart));

            res = ReadFile(file, fileData.data(), (DWORD)fileData.size(), &readed, nullptr);

            CloseHandle(file);
        }

        if (res == FALSE) {
            return std::vector<uint8_t>();
        }
        else {
            return fileData;
        }
    }

    std::wstring System::GetPackagePath() {
        std::wstring packagePath;
#if HAVE_WINRT == 1
        auto tmpPath = Windows::ApplicationModel::Package::Current->InstalledLocation->Path;
        std::wstring installedPath(tmpPath->Data(), tmpPath->Length());
        packagePath = installedPath + L"\\";
#else
        WCHAR exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        packagePath = exePath;
        packagePath = packagePath.substr(0, packagePath.find_last_of('\\') + 1);
#endif
        return packagePath;
    }

    std::wstring System::ExePathW() {
        wchar_t buffer[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, buffer, MAX_PATH);
        auto pos = std::wstring{ buffer }.find_last_of(L"\\/");
        return std::wstring{ buffer }.substr(0, pos);
    }
    std::string System::ExePathA() {
        char buffer[MAX_PATH] = { 0 };
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        auto pos = std::string{ buffer }.find_last_of("\\/");
        return std::string{ buffer }.substr(0, pos);
    }

#if HAVE_WINRT == 1
    HRESULT System::PerformSync(Windows::Foundation::IAsyncAction ^op) {
        std::promise<HRESULT> prom;
        auto fut = prom.get_future();

        op->Completed = ref new Windows::Foundation::AsyncActionCompletedHandler(
            [&](Windows::Foundation::IAsyncAction ^op, Windows::Foundation::AsyncStatus status) {
            HRESULT res;

            if (status == Windows::Foundation::AsyncStatus::Completed) {
                res = S_OK;
            }
            else {
                res = static_cast<HRESULT>(op->ErrorCode.Value);
            }

            prom.set_value(res);
        });

        auto result = fut.get();

        return result;
    }

    void System::PerformSyncThrow(Windows::Foundation::IAsyncAction ^op) {
        System::ThrowIfFailed(System::PerformSync(op));
    }
#endif
}