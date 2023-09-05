#pragma once
#include "HWindows.h"
//#include <MagicEnum/MagicEnum.h>

#include <condition_variable>
#include <functional>
#include <string>
#include <vector>
#include <thread>
#include <sddl.h>
#include <queue>

#include "Thread.h"
//#include "Logger.h"
#include "Helpers.h"
#include "LocalPtr.hpp"


#define LogDebugWithFullClassNameA(...)
#define LogDebugWithFullClassNameW(...)

#define LogWarningWithFullClassNameA(...)
#define LogWarningWithFullClassNameW(...)

#define LogErrorWithFullClassNameA(...)
#define LogErrorWithFullClassNameW(...)

#define LOG_FUNCTION_ENTER(...)

#define LOG_DEBUG(...)
#define LOG_ERROR(...)

#define LOG_DEBUG_D(...)
#define LOG_ERROR_D(...)

#define LogLastError


enum class PipeConnectionStatus {
    Error,
    Stopped,
    TimeoutConnection,
    Connected,
};

enum class PipeError {
    InvalidHandle,
    WriteError,
    ReadError,
};

PipeConnectionStatus WaitConnectPipe(IN HANDLE hPipe, const std::atomic<bool>& stop, int timeout = 0);
PipeConnectionStatus WaitOpenPipe(OUT HANDLE& hPipe, const std::wstring& pipeName, const std::atomic<bool>& stop, int timeout = 0);

// TODO: move to file system helpers
// TODO: rewrite this without blocking current thread
template<typename T = uint8_t>
std::vector<T> ReadFileAsync(HANDLE hFile, const std::atomic<bool>& stop) {
    enum StatusIO {
        Error,
        Stopped,
        Completed,
    };
    StatusIO status;

    DWORD dwBytesRead = 0;
    OVERLAPPED stOverlapped = { 0 };
    std::vector<T> tmpBuffer(1024 * 1024 * 2); // NOTE: for simplicity we can use max buffer size ... 2 MB

    if (ReadFile(hFile, tmpBuffer.data(), tmpBuffer.size() * sizeof(T), &dwBytesRead, &stOverlapped)) {
        // ReadFile completed synchronously ...
        status = StatusIO::Completed;
    }
    else {
        auto lastErrorReadFile = GetLastError();
        switch (lastErrorReadFile) {
        case ERROR_IO_PENDING:
            while (true) {
                // Check break conditions in while scope (if we chek it after while may be status conflicts)
                if (stop) { 
                    status = StatusIO::Stopped;
                    break;
                }

                if (GetOverlappedResult(hFile, &stOverlapped, &dwBytesRead, FALSE)) {
                    // ReadFile operation completed
                    status = StatusIO::Completed;
                    break;
                }
                else {
                    // pending ...
                    auto lastErrorGetOverlappedResult = GetLastError();
                    if (lastErrorGetOverlappedResult != ERROR_IO_INCOMPLETE) {
                        // remote side was closed or smth else
                        status = StatusIO::Error;
                        LogLastError;
                        break;
                    }
                }
                Sleep(1); // TODO: for capturing frame use less delay (CHECK)
            }
            break;

        default:
            LogLastError;
            status = StatusIO::Error;
        }
    }
  
    switch (status) {
    case StatusIO::Error:
        throw std::exception("ReadFile error");
        break;

    case StatusIO::Stopped:
        throw std::exception("Was stopped signal");
        break;

    case StatusIO::Completed:
        if (dwBytesRead == 0) {
            throw std::exception("Zero data was read");
        }
    }
    
    tmpBuffer.resize(dwBytesRead / sizeof(T)); // truncate
    return tmpBuffer;
}


template <typename T = uint8_t>
void WriteToPipe(HANDLE hNamedPipe, const std::vector<T>& writeData) {
    DWORD cbWritten;
    if (!WriteFile(hNamedPipe, writeData.data(), writeData.size() * sizeof(T), &cbWritten, NULL)) { // Write synchronously
        throw PipeError::WriteError;
    }
}

template <typename T = uint8_t>
std::vector<T> ReadFromPipeAsync(HANDLE hNamedPipe, const std::atomic<bool>& stop) {
    try {
        return ReadFileAsync<T>(hNamedPipe, stop);
    }
    catch (...) {
        throw PipeError::ReadError;
    }

}


template<typename T>
H::LocalPtr<T> GetTokenInfo(HANDLE hToken, TOKEN_INFORMATION_CLASS typeInfo) {
    DWORD dwSize = 0;
    if (!GetTokenInformation(hToken, typeInfo, NULL, 0, &dwSize) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        return nullptr;
    }

    if (auto pTokenData = H::LocalPtr<T>(LocalAlloc(LPTR, dwSize))) {
        if (GetTokenInformation(hToken, typeInfo, pTokenData.get(), dwSize, &dwSize)) {
            return pTokenData;
        }
    }

    return nullptr;
}


// NOTE: EnumMsg must contain "Connect" msg and first "None" msg (fix in future)
// TODO: Add guard for multiple calls pulbic methods and for usage in them in multithreading
// TODO: fix when interrupt connection -> not process "None" msg (??? CHECK)
template<typename EnumMsg, typename T = uint8_t>
class Channel {
    //CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(Channel);

public:
    struct Reply {
        std::vector<T> readData;
        EnumMsg type;
    };
    using ReadFunc = std::function<Reply()>;
    using WriteFunc = std::function<void(std::vector<T>&&, EnumMsg)>;

    Channel() = default;
    ~Channel() {
        StopChannel();
        if (hNamedPipe) {
            CloseHandle(hNamedPipe);
        }
    }

    // Make security attributes to connect admin & user pipe
    void CreateForAdmin(const std::wstring& pipeName, std::function<bool(ReadFunc, WriteFunc)> listenHandler, int timeout = 0) {
        LogDebugWithFullClassNameW(L"CreateForAdmin(pipeName = {}, ...) ...", pipeName);

        auto pSecurityDescriptor = std::make_unique<SECURITY_DESCRIPTOR>();
        InitializeSecurityDescriptor(pSecurityDescriptor.get(), SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(pSecurityDescriptor.get(), TRUE, (PACL)NULL, FALSE);
        pSecurityAttributes = std::make_unique<SECURITY_ATTRIBUTES>(SECURITY_ATTRIBUTES{ sizeof(SECURITY_ATTRIBUTES), pSecurityDescriptor.get(), FALSE });

        Create(pipeName, listenHandler, timeout);
    }

    // Make security attributes to connect UWP & desktop apps
    void CreateForUWP(const std::wstring& pipeName, std::function<bool(ReadFunc, WriteFunc)> listenHandler, HANDLE hProcessUWP, int timeout = 0) {
        LogDebugWithFullClassNameW(L"CreateForUWP(pipeName = {}, ...) ...", pipeName);

        pSecurityAttributes = std::make_unique<SECURITY_ATTRIBUTES>(SECURITY_ATTRIBUTES{ sizeof(SECURITY_ATTRIBUTES), NULL, FALSE });

        // SDDL_EVERYONE + SDDL_ALL_APP_PACKAGES + SDDL_ML_LOW
        if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(L"D:(A;;GA;;;WD)(A;;GA;;;AC)S:(ML;;;;;LW)", SDDL_REVISION_1, &pSecurityAttributes->lpSecurityDescriptor, 0)) {
            LogLastError;
            throw PipeError::InvalidHandle;
        }

        std::wstring formattedPipeName;

        HANDLE hToken = nullptr;
        if (OpenProcessToken(hProcessUWP, TOKEN_QUERY, &hToken)) {
            if (auto pTokenAppContainerInfo = GetTokenInfo<TOKEN_APPCONTAINER_INFORMATION>(hToken, ::TokenAppContainerSid)) {
                if (auto pTokenSessingId = GetTokenInfo<ULONG>(hToken, ::TokenSessionId)) {
                    H::LocalPtr<WCHAR> pStr;
                    if (ConvertSidToStringSidW(pTokenAppContainerInfo->TokenAppContainer, pStr.ReleaseAndGetAdressOf())) {
                        formattedPipeName = L"\\\\?\\pipe\\Sessions\\" + std::to_wstring(*pTokenSessingId) + L"\\AppContainerNamedObjects\\" + pStr.get() + L"\\" + pipeName;
                    }
                    else {
                        LogLastError;
                        throw PipeError::InvalidHandle;
                    }
                }
            }
        }

        Create(formattedPipeName, listenHandler, timeout);
    }

    void Create(const std::wstring& pipeName, std::function<bool(ReadFunc, WriteFunc)> listenHandler, int timeout = 0) {
        LogDebugWithFullClassNameW(L"Create(pipeName = {}, listenHandler, timeout = {}) ...", pipeName, timeout);

        hNamedPipe = CreateNamedPipeW(
            pipeName.c_str(),
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            512, 512, 5000, pSecurityAttributes.get());

        if (hNamedPipe == INVALID_HANDLE_VALUE) {
            throw PipeError::InvalidHandle;
        }

        closeChannel = false;
        threadChannel = std::thread([this, listenHandler, timeout, pipeName] {
            H::ThreadNameHelper::SetThreadName(pipeName + L" Create thread");

            while (!closeChannel) {
                LogDebugWithFullClassNameA("Waiting for connect...");
                auto status = WaitConnectPipe(hNamedPipe, closeChannel, timeout);
                LogDebugWithFullClassNameA("WaitConnectPipe status = {}", magic_enum::enum_name(status));

                switch (status) {
                case PipeConnectionStatus::Connected: {
                    ListenRoutine(listenHandler);
                    break;
                }
                case PipeConnectionStatus::Error:
                case PipeConnectionStatus::Stopped:
                case PipeConnectionStatus::TimeoutConnection:
                    closeChannel = true;
                    stopSignal = true;
                    break;
                }

                StopSecondThreads();
                DisconnectNamedPipe(hNamedPipe);
            }
            });
    }



    void Open(const std::wstring& pipeName, std::function<bool(ReadFunc, WriteFunc)> listenHandler, int timeout = 10'000) {
        LogDebugWithFullClassNameA("Open() ...");

        closeChannel = false;
        auto status = WaitOpenPipe(hNamedPipe, pipeName, closeChannel, timeout);
        LogDebugWithFullClassNameA("WaitOpenPipe status = {}", magic_enum::enum_name(status));

        switch (status) {
        case PipeConnectionStatus::Connected: {
            break;
        }
        case PipeConnectionStatus::Error:
        case PipeConnectionStatus::Stopped:
        case PipeConnectionStatus::TimeoutConnection:
            throw PipeError::InvalidHandle;
            break;
        }

        threadChannel = std::thread([this, listenHandler, pipeName] {
            H::ThreadNameHelper::SetThreadName(pipeName + L" Open thread");
            ListenRoutine(listenHandler);
            });
    }


    void StopChannel() {
        LogDebugWithFullClassNameA("StopChannel() ...");

        connected = false;
        stopSignal = true;
        closeChannel = true;

        if (threadChannel.joinable()) // wait finish channel thread where init interrupt thread 
            threadChannel.join();

        StopSecondThreads();
    }

    bool IsConnected() {
        return connected;
    }


    void SetInterruptHandler(std::function<void()> handler) {
        interruptHandler = handler;
    }

    void SetConnectHandler(std::function<void()> handler) {
        connectHandler = handler;
    }


    void ClearPendingMessages() {
        LogDebugWithFullClassNameA("ClearPendingMessages() ...");
        std::lock_guard lk{ mxWrite };
        pendingMessages.clear();
    }

    void WritePendingMessages() {
        LogDebugWithFullClassNameA("WritePendingMessages() ...");
        std::lock_guard lk{ mxWrite };
        try {
            for (auto& message : pendingMessages) {
                WriteToPipe<T>(hNamedPipe, message);
                if (waitedMessage != EnumMsg::None && waitedMessage == static_cast<EnumMsg>(message.back())) {
                    cvFinishSendingMessage.notify_all();
                }
            }
            pendingMessages.clear();
        }
        catch (PipeError error) {
            switch (error)
            {
            case PipeError::WriteError:
                LogErrorWithFullClassNameA("Write error! Stop channel.");
                break;
            };
            stopSignal = true;
        }
    }

    // Write synchronously
    void Write(std::vector<T>&& writeData, EnumMsg type) {
        std::unique_lock lk{ mxWrite };

        writeData.push_back(static_cast<T>(type));
        if (!connected) {
            pendingMessages.push_back(std::move(writeData));
            return;
        }

        try {
            WriteToPipe<T>(hNamedPipe, writeData);
            if (waitedMessage != EnumMsg::None && waitedMessage == type) {
                cvFinishSendingMessage.notify_all();
            }
        }
        catch (PipeError error) {
            switch (error)
            {
            case PipeError::WriteError:
                LogErrorWithFullClassNameA("Write error! Stop channel.");
                break;
            };
            stopSignal = true;
        }
    }

    // TODO: add logic to wait for one of several messages
    void WaitFinishSendingMessage(EnumMsg type) {
        std::unique_lock lk{ mxWrite };

        if (hNamedPipe == INVALID_HANDLE_VALUE)
            return;

        waitedMessage = type;
        cvFinishSendingMessage.wait(lk); // now mutex unlocked and thread begin wait
        waitedMessage = EnumMsg::None;
    }

private:
    void ListenRoutine(std::function<bool(ReadFunc, WriteFunc)> listenHandler) {
        LogDebugWithFullClassNameA("ListenRoutine() ...");

        connected = true;
        stopSignal = false;

        threadConnect = std::thread([this] { // Not block current thread to avoid deadlock (TODO: replace on std::async)
            connectHandler();
            });

        Write({}, EnumMsg::Connect);

        while (!stopSignal) {
            if (listenHandler(bindedReadFunc, bindedWriteFunc) == false) {
                stopSignal = true;
            }
        }
        LogDebugWithFullClassNameA("stopSignal = true");

        threadInterrupt = std::thread([this] { // Not block current thread to avoid deadlock
            interruptHandler();
            });

        cvFinishSendingMessage.notify_all();
        connected = false; // if we here pipe not connected
    }

    Reply Read() {
        try {
            auto readData = ReadFromPipeAsync<T>(hNamedPipe, stopSignal);
            if (readData.size() > 0) {
                EnumMsg type = static_cast<EnumMsg>(readData.back());
                readData.pop_back();
                return Reply{ std::move(readData), type };
            }
        }
        catch (PipeError error) {
            switch (error)
            {
            case PipeError::ReadError:
                LogErrorWithFullClassNameA("Write error! Stop channel.");
                break;
            };
            stopSignal = true;
        }
        return {};
    }

    void StopSecondThreads() {
        if (threadConnect.joinable())
            threadConnect.join();

        if (threadInterrupt.joinable())
            threadInterrupt.join();
    }


private:
    std::mutex mxWrite;
    HANDLE hNamedPipe = nullptr;
    std::thread threadChannel;
    std::thread threadConnect;
    std::thread threadInterrupt;
    std::atomic<bool> connected = false;
    std::atomic<bool> stopSignal = false;
    std::atomic<bool> closeChannel = false;
    std::function<void()> connectHandler = []() {};
    std::function<void()> interruptHandler = []() {};
    std::vector<std::vector<T>> pendingMessages;

    std::atomic<EnumMsg> waitedMessage = EnumMsg::None;
    std::condition_variable cvFinishSendingMessage;
    std::unique_ptr<SECURITY_ATTRIBUTES> pSecurityAttributes;

    const ReadFunc bindedReadFunc = std::bind(&Channel::Read, this);
    const WriteFunc bindedWriteFunc = std::bind(&Channel::Write, this, std::placeholders::_1, std::placeholders::_2);
};