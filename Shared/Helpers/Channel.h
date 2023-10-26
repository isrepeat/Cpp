#pragma once
#define NEW_CHANNEL 2

#if NEW_CHANNEL == 2
#include "HWindows.h"
#include <MagicEnum/MagicEnum.h>

#include <condition_variable>
#include <functional>
#include <string>
#include <vector>
#include <thread>
#include <sddl.h>
#include <queue>
#include <span>

#include "Thread.h"
#include "HLogger.h"
#include "Helpers.h"
#include "Conversion.h"
#include "LocalPtr.hpp"


#if !defined(LOG_FUNCTION_ENTER_READ)
#   define LOG_FUNCTION_ENTER_READ(...) LOG_FUNCTION_ENTER(__VA_ARGS__)
#endif
#if !defined(LOG_DEBUG_READ)
#   define LOG_DEBUG_READ(...) LOG_DEBUG(__VA_ARGS__)
#endif

#if !defined(LOG_FUNCTION_ENTER_WRITE)
#   define LOG_FUNCTION_ENTER_WRITE(...) LOG_FUNCTION_ENTER(__VA_ARGS__)
#endif
#if !defined(LOG_DEBUG_WRITE)
#   define LOG_DEBUG_WRITE(...) LOG_DEBUG(__VA_ARGS__)
#endif


#define PIPE_BUFFER 24


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
//void WriteToPipe(HANDLE hNamedPipe, const std::vector<T>& writeData) {
void WriteToPipe(HANDLE hNamedPipe, std::span<T> writeData) {
    DWORD cbWritten;
    do {
        if (!WriteFile(hNamedPipe, writeData.data(), writeData.size() * sizeof(T), &cbWritten, NULL)) { // Write synchronously
            throw PipeError::WriteError;
        }
    } while (cbWritten < writeData.size() * sizeof(T));
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
    CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(Channel);

public:

    struct Message {
        EnumMsg type;
        std::vector<T> payload;
    };

    struct MessageInternal {
        bool sizeWasRead = false;
        bool typeWasRead = false;
        bool payloadWasRead = false;
        uint32_t size = 0;
        Message message;
    };


    //struct ReadMessage {
    //    EnumMsg type;
    //    std::vector<T> msgData;
    //};

    //struct WriteMessage {
    //    WriteMessage(EnumMsg type, std::vector<T>&& msgData)
    //        : type{ type }ss
    //        , msgData(msgData.size() + 10)
    //    {
    //        this->msgData = std::move(msgData);
    //        this->msgData[msgData.size()] = static_cast<T>(type);
    //        int xxx = 9;
    //    }

    //    EnumMsg GetType() const {
    //        return type;
    //    }

    //    const std::vector<T>& GetMsg() const {
    //        return msgData;
    //    }

    //private:
    //    EnumMsg type;
    //    std::vector<T> msgData;
    //};

    using ReadFunc = std::function<Message()>;
    using WriteFunc = std::function<void(std::vector<T>&&, EnumMsg)>;
    //using WriteFunc = std::function<void(const std::vector<T>&, EnumMsg)>;

    Channel() = default;
    ~Channel() {
        StopChannel();
        if (hNamedPipe) {
            CloseHandle(hNamedPipe);
        }
    }

    // Make security attributes to connect admin & user pipe
    void CreateForAdmin(const std::wstring& pipeName, std::function<bool(ReadFunc, WriteFunc)> listenHandler, int timeout = 0) {
        LOG_DEBUG(L"CreateForAdmin(pipeName = {}, ...) ...", pipeName);

        auto pSecurityDescriptor = std::make_unique<SECURITY_DESCRIPTOR>();
        InitializeSecurityDescriptor(pSecurityDescriptor.get(), SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(pSecurityDescriptor.get(), TRUE, (PACL)NULL, FALSE);
        pSecurityAttributes = std::make_unique<SECURITY_ATTRIBUTES>(SECURITY_ATTRIBUTES{ sizeof(SECURITY_ATTRIBUTES), pSecurityDescriptor.get(), FALSE });

        Create(pipeName, listenHandler, timeout);
    }

    // Make security attributes to connect UWP & desktop apps
    void CreateForUWP(const std::wstring& pipeName, std::function<bool(ReadFunc, WriteFunc)> listenHandler, HANDLE hProcessUWP, int timeout = 0) {
        LOG_DEBUG(L"CreateForUWP(pipeName = {}, ...) ...", pipeName);

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
        LOG_DEBUG(L"Create(pipeName = {}, listenHandler, timeout = {}) ...", pipeName, timeout);

        hNamedPipe = CreateNamedPipeW(
            pipeName.c_str(),
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            24, 24, 5000, pSecurityAttributes.get());

        if (hNamedPipe == INVALID_HANDLE_VALUE) {
            throw PipeError::InvalidHandle;
        }

        closeChannel = false;
        threadChannel = std::thread([this, listenHandler, timeout, pipeName] {
            //LOG_THREAD(pipeName + L" Create thread");

            while (!closeChannel) {
                LOG_DEBUG("Waiting for connect...");
                auto status = WaitConnectPipe(hNamedPipe, closeChannel, timeout);
                LOG_DEBUG("WaitConnectPipe status = {}", magic_enum::enum_name(status));

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
        LOG_DEBUG("Open() ...");

        closeChannel = false;
        auto status = WaitOpenPipe(hNamedPipe, pipeName, closeChannel, timeout);
        LOG_DEBUG("WaitOpenPipe status = {}", magic_enum::enum_name(status));

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
            //LOG_THREAD(pipeName + L" Open thread");
            ListenRoutine(listenHandler);
            });
    }


    void StopChannel() {
        LOG_DEBUG("StopChannel() ...");

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
        LOG_DEBUG("ClearPendingMessages() ...");
        std::lock_guard lk{ mxWrite };
        pendingMessages.clear();
    }


    void WritePendingMessages() {
        //LOG_DEBUG("WritePendingMessages() ...");
        //std::lock_guard lk{ mxWrite };
        //try {
        //    for (auto& message : pendingMessages) {
        //        WriteToPipe<T>(hNamedPipe, message);
        //        if (waitedMessage != EnumMsg::None && waitedMessage == static_cast<EnumMsg>(message.back())) {
        //            cvFinishSendingMessage.notify_all();
        //        }
        //    }
        //    pendingMessages.clear();
        //}
        //catch (PipeError error) {
        //    switch (error)
        //    {
        //    case PipeError::WriteError:
        //        LOG_ERROR_D("Write error! Stop channel.");
        //        break;
        //    };
        //    stopSignal = true;
        //}
    }


    //void Write(const WriteMessage& message) {
    //    std::unique_lock lk{ mxWrite };

    //    if (!connected) {
    //        //pendingMessages.push_back(std::move(writeData));
    //        return;
    //    }

    //    try {
    //        WriteToPipe<T>(hNamedPipe, message.GetMsg());
    //        if (waitedMessage != EnumMsg::None && waitedMessage == message.GetType()) {
    //            cvFinishSendingMessage.notify_all();
    //        }
    //    }
    //    catch (PipeError error) {
    //        switch (error)
    //        {
    //        case PipeError::WriteError:
    //            LOG_ERROR_D("Write error! Stop channel.");
    //            break;
    //        };
    //        stopSignal = true;
    //    }
    //}


    void Write(std::vector<T>&& writeData, EnumMsg type) {
        std::unique_lock lk{ mxWrite };
        LOG_FUNCTION_ENTER_WRITE("Write(writeData, type)");


        if (!connected) {
            //pendingMessages.push_back(std::move(writeData));
            return;
        }

        try {
            T messageSizeBuffer[sizeof(uint32_t)];
            T messageTypeBuffer[sizeof(uint32_t)];
            uint32_t messageSize = writeData.size();
            uint32_t messageType = static_cast<uint8_t>(type);
            std::memcpy(&messageSizeBuffer, &messageSize, sizeof(uint32_t));
            std::memcpy(&messageTypeBuffer, &messageType, sizeof(uint32_t));

            WriteToPipe<T>(hNamedPipe, messageSizeBuffer);
            LOG_DEBUG_WRITE("write size = {} '{}'", messageSize, BytesRangeToHexString(messageSizeBuffer, 0, 4));
            WriteToPipe<T>(hNamedPipe, messageTypeBuffer);
            LOG_DEBUG_WRITE("write type = {} '{}'", messageType, BytesRangeToHexString(messageTypeBuffer, 0, 4));
            WriteToPipe<T>(hNamedPipe, writeData);
            int countPayloadBytes = messageSize < 20 ? messageSize : 20;
            LOG_DEBUG_WRITE("write payload = {} '{}'", messageSize, BytesRangeToHexString(writeData, 0, countPayloadBytes));

            if (waitedMessage != EnumMsg::None && waitedMessage == type) {
                cvFinishSendingMessage.notify_all();
            }
        }
        catch (PipeError error) {
            switch (error)
            {
            case PipeError::WriteError:
                LOG_ERROR_D("Write error! Stop channel.");
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
        LOG_DEBUG("ListenRoutine() ...");

        connected = true;
        stopSignal = false;

        threadConnect = std::thread([this] { // Not block current thread to avoid deadlock (TODO: replace on std::async)
            connectHandler();
            });


        //Write({}, EnumMsg::Connect);

        while (!stopSignal) {
            if (listenHandler(bindedReadFunc, bindedWriteFunc) == false) {
                stopSignal = true;
            }
        }
        LOG_DEBUG("stopSignal = true");

        threadInterrupt = std::thread([this] { // Not block current thread to avoid deadlock
            interruptHandler();
            });

        cvFinishSendingMessage.notify_all();
        connected = false; // if we here pipe not connected
    }

    Message Read() {
        LOG_FUNCTION_ENTER_READ("Read()");
        try {
            bool haveAtLeastOneCompletedMessage = false;
            do {
                auto readBuffer = ReadFromPipeAsync<T>(hNamedPipe, stopSignal);
                uint32_t processedBytes = 0;
                LOG_DEBUG_READ("readBuffer size = {}", readBuffer.size());

                if (readMessages.size() > 0) {
                    LOG_DEBUG_READ("Finish parse last previous message");
                    ParseMessage(readBuffer, processedBytes, readMessages.back()); // Finish parse last previous message
                }
                while (readBuffer.size() > processedBytes) {
                    MessageInternal messageInternal;
                    ParseMessage(readBuffer, processedBytes, messageInternal);
                    readMessages.push(std::move(messageInternal));
                }
                if (readMessages.size() > 0) {
                    auto& msg = readMessages.front();
                    haveAtLeastOneCompletedMessage = msg.sizeWasRead && msg.typeWasRead && msg.payloadWasRead;
                }
            } while (!haveAtLeastOneCompletedMessage);

            auto firstInternalMessage = std::move(readMessages.front());
            readMessages.pop();
            return firstInternalMessage.message;
        }
        catch (PipeError error) {
            switch (error)
            {
            case PipeError::ReadError:
                LOG_ERROR_D("Write error! Stop channel.");
                break;
            };
            stopSignal = true;
        }
        return {};
    }

    void ParseMessage(const std::vector<T>& readBuffer, uint32_t& processedBytes, MessageInternal& messageInternal) {
        LOG_FUNCTION_ENTER_READ("ParseMessage(readBuffer [{}], processedBytes = {}, messageInternal)", readBuffer.size(), processedBytes);

        if (!messageInternal.sizeWasRead && readBuffer.size() > processedBytes) {
            messageInternal.size = *reinterpret_cast<const uint32_t*>(readBuffer.data() + processedBytes);
            LOG_DEBUG_READ("msg size = {} '{}'", messageInternal.size, BytesRangeToHexString(const_cast<std::vector<T>&>(readBuffer), processedBytes, 4));
            processedBytes += sizeof(uint32_t);
            messageInternal.sizeWasRead = true;
        }
        if (!messageInternal.typeWasRead && readBuffer.size() > processedBytes) {
            auto type = *reinterpret_cast<const uint32_t*>(readBuffer.data() + processedBytes);
            LOG_DEBUG_READ("msg type = {} '{}'", type, BytesRangeToHexString(const_cast<std::vector<T>&>(readBuffer), processedBytes, 4));
            messageInternal.message.type = static_cast<EnumMsg>(type);
            processedBytes += sizeof(uint32_t);
            messageInternal.typeWasRead = true;
        }
        if (!messageInternal.payloadWasRead && readBuffer.size() > processedBytes) {
            int countPayloadBytes = messageInternal.size < (readBuffer.size() - processedBytes) ? messageInternal.size : (readBuffer.size() - processedBytes);
            LOG_DEBUG_READ("msg payload [start = {}, end = {}] '{}'", processedBytes, processedBytes + messageInternal.size, BytesRangeToHexString(const_cast<std::vector<T>&>(readBuffer), processedBytes, countPayloadBytes));
            messageInternal.message.payload.insert(messageInternal.message.payload.end(), readBuffer.begin() + processedBytes, readBuffer.begin() + processedBytes + messageInternal.size);
            processedBytes += messageInternal.size;
            messageInternal.payloadWasRead = true;
        }
    }


    std::string BytesRangeToHexString(std::span<T> bytes, int idxStart, int count) {
        std::string bytesStr(count * 3, '\0'); // 2 char, 1 space

        std::span<char> spanBytesStr = bytesStr;
        auto it = spanBytesStr.begin();
        const auto itEnd = spanBytesStr.end();

        for (int i = 0; i < count; i++) {
            it = H::HexByte(bytes[idxStart + i], it, itEnd);
            *it++ = ' ';
        }

        return bytesStr;
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
    std::queue<MessageInternal> readMessages;

    std::atomic<EnumMsg> waitedMessage = EnumMsg::None;
    std::condition_variable cvFinishSendingMessage;
    std::unique_ptr<SECURITY_ATTRIBUTES> pSecurityAttributes;

    const ReadFunc bindedReadFunc = std::bind(&Channel::Read, this);
    const WriteFunc bindedWriteFunc = std::bind(&Channel::Write, this, std::placeholders::_1, std::placeholders::_2);
};
#elif NEW_CHANNEL == 1
#include "HWindows.h"
#include <MagicEnum/MagicEnum.h>

#include <condition_variable>
#include <functional>
#include <string>
#include <vector>
#include <thread>
#include <sddl.h>
#include <queue>
#include <span>

#include "Thread.h"
#include "HLogger.h"
#include "Helpers.h"
#include "LocalPtr.hpp"


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
    //void WriteToPipe(HANDLE hNamedPipe, std::span<T> writeData) {
    DWORD cbWritten;
    do {
        if (!WriteFile(hNamedPipe, writeData.data(), writeData.size() * sizeof(T), &cbWritten, NULL)) { // Write synchronously
            throw PipeError::WriteError;
        }
    } while (cbWritten < writeData.size() * sizeof(T));
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
    CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(Channel);

public:
    struct Message {
        EnumMsg type;
        std::vector<T> msgData;
    };

    //struct Message {
    //    EnumMsg type;
    //    int msgSize;
    //    uint8_t* msgData;
    //};

    using ReadFunc = std::function<Message()>;
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
        LOG_DEBUG(L"CreateForAdmin(pipeName = {}, ...) ...", pipeName);

        auto pSecurityDescriptor = std::make_unique<SECURITY_DESCRIPTOR>();
        InitializeSecurityDescriptor(pSecurityDescriptor.get(), SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(pSecurityDescriptor.get(), TRUE, (PACL)NULL, FALSE);
        pSecurityAttributes = std::make_unique<SECURITY_ATTRIBUTES>(SECURITY_ATTRIBUTES{ sizeof(SECURITY_ATTRIBUTES), pSecurityDescriptor.get(), FALSE });

        Create(pipeName, listenHandler, timeout);
    }

    // Make security attributes to connect UWP & desktop apps
    void CreateForUWP(const std::wstring& pipeName, std::function<bool(ReadFunc, WriteFunc)> listenHandler, HANDLE hProcessUWP, int timeout = 0) {
        LOG_DEBUG(L"CreateForUWP(pipeName = {}, ...) ...", pipeName);

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
        LOG_DEBUG(L"Create(pipeName = {}, listenHandler, timeout = {}) ...", pipeName, timeout);

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
            //LOG_THREAD(pipeName + L" Create thread");

            while (!closeChannel) {
                LOG_DEBUG("Waiting for connect...");
                auto status = WaitConnectPipe(hNamedPipe, closeChannel, timeout);
                LOG_DEBUG("WaitConnectPipe status = {}", magic_enum::enum_name(status));

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
        LOG_DEBUG("Open() ...");

        closeChannel = false;
        auto status = WaitOpenPipe(hNamedPipe, pipeName, closeChannel, timeout);
        LOG_DEBUG("WaitOpenPipe status = {}", magic_enum::enum_name(status));

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
            //LOG_THREAD(pipeName + L" Open thread");
            ListenRoutine(listenHandler);
            });
    }


    void StopChannel() {
        LOG_DEBUG("StopChannel() ...");

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
        LOG_DEBUG("ClearPendingMessages() ...");
        std::lock_guard lk{ mxWrite };
        pendingMessages.clear();
    }


    void WritePendingMessages() {
        LOG_DEBUG("WritePendingMessages() ...");
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
                LOG_ERROR_D("Write error! Stop channel.");
                break;
            };
            stopSignal = true;
        }
    }

    void Write(std::vector<T>&& writeData, EnumMsg type) {
        std::unique_lock lk{ mxWrite };

        if (writeData.empty()) {
            writeData.push_back(static_cast<T>(type));
        }
        else {
            writeData.back() = (static_cast<T>(type));
        }

        if (!connected) {
            //pendingMessages.push_back(std::move(writeData));
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
                LOG_ERROR_D("Write error! Stop channel.");
                break;
            };
            stopSignal = true;
        }
    }



    //void WritePendingMessages() {
    //    LOG_DEBUG("WritePendingMessages() ...");
    //    std::lock_guard lk{ mxWrite };
    //    try {
    //        for (auto& message : pendingMessages) {
    //            auto ptrMessage = reinterpret_cast<uint8_t*>(&message);
    //            auto spanMessage = std::span<uint8_t>(ptrMessage, sizeof(Message));

    //            WriteToPipe<T>(hNamedPipe, spanMessage);
    //            if (waitedMessage != EnumMsg::None && waitedMessage == message.type) {
    //                cvFinishSendingMessage.notify_all();
    //            }
    //        }
    //        pendingMessages.clear();
    //    }
    //    catch (PipeError error) {
    //        switch (error)
    //        {
    //        case PipeError::WriteError:
    //            LOG_ERROR_D("Write error! Stop channel.");
    //            break;
    //        };
    //        stopSignal = true;
    //    }
    //}

    //// Write synchronously
    //void Write(std::vector<T>&& writeData, EnumMsg type) {
    //    std::unique_lock lk{ mxWrite };
    //    
    //    try {
    //        uint8_t* storage = new uint8_t[sizeof(Message) + writeData.size()]{};
    //        Message* message = reinterpret_cast<Message*>(storage);
    //        message->type = type;
    //        message->msgSize = writeData.size();
    //        message->msgData = storage + sizeof(Message);
    //        std::copy(writeData.begin(), writeData.end(), storage + sizeof(Message));


    //        if (!connected) {
    //            //pendingMessages.push_back(std::move(*message));
    //            return;
    //        }

    //        auto spanMessage = std::span<uint8_t>(storage, sizeof(Message) + writeData.size());

    //        WriteToPipe<T>(hNamedPipe, spanMessage);
    //        if (waitedMessage != EnumMsg::None && waitedMessage == type) {
    //            cvFinishSendingMessage.notify_all();
    //        }

    //        delete[] storage;
    //    }
    //    catch (PipeError error) {
    //        switch (error)
    //        {
    //        case PipeError::WriteError:
    //            LOG_ERROR_D("Write error! Stop channel.");
    //            break;
    //        };
    //        stopSignal = true;
    //    }
    //}

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
        LOG_DEBUG("ListenRoutine() ...");

        connected = true;
        stopSignal = false;

        threadConnect = std::thread([this] { // Not block current thread to avoid deadlock (TODO: replace on std::async)
            connectHandler();
            });


        //Write({}, EnumMsg::Connect);

        while (!stopSignal) {
            if (listenHandler(bindedReadFunc, bindedWriteFunc) == false) {
                stopSignal = true;
            }
        }
        LOG_DEBUG("stopSignal = true");

        threadInterrupt = std::thread([this] { // Not block current thread to avoid deadlock
            interruptHandler();
            });

        cvFinishSendingMessage.notify_all();
        connected = false; // if we here pipe not connected
    }

    Message Read() {
        try {
            auto readData = ReadFromPipeAsync<T>(hNamedPipe, stopSignal);
            if (readData.size() > 0) {
                EnumMsg type = static_cast<EnumMsg>(readData.back());
                readData.pop_back();
                return Message{ type, std::move(readData) };
            }
        }
        catch (PipeError error) {
            switch (error)
            {
            case PipeError::ReadError:
                LOG_ERROR_D("Write error! Stop channel.");
                break;
            };
            stopSignal = true;
        }
        return {};
    }

    //Message Read() {
    //    try {
    //        auto readData = ReadFromPipeAsync<T>(hNamedPipe, stopSignal);
    //        if (readData.size() > 0) {
    //            //EnumMsg type = static_cast<EnumMsg>(readData.back());
    //            //readData.pop_back();
    //            //return Message{ std::move(readData), type };
    //            
    //            Message message;
    //            auto ptrMessage = reinterpret_cast<uint8_t*>(&message);
    //            std::copy(readData.begin(), readData.end(), ptrMessage);
    //            message.msgData = readData.data() + sizeof(Message);

    //            return message;
    //            //return *reinterpret_cast<Message*>(readData.data());
    //        }
    //    }
    //    catch (PipeError error) {
    //        switch (error)
    //        {
    //        case PipeError::ReadError:
    //            LOG_ERROR_D("Write error! Stop channel.");
    //            break;
    //        };
    //        stopSignal = true;
    //    }
    //    return {};
    //}

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
#endif