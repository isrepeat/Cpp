#pragma once
#include <Windows.h>
#include <functional>
#include <string>
#include <vector>
#include <thread>
#include <condition_variable>
#include <queue>

// TODO: add TimeoutConnection exception !!!!!!!!

enum class PipeError {
    //TimeoutConnection,
    //PipeDisconnected,
    InvalidHandle,
    WriteError,
    ReadError,
};

std::exception GetLastErrorException(const std::string& msg);
bool WaitConnectPipe(HANDLE hPipe, const std::atomic<bool>& stop, int timeout = 0);
HANDLE WaitOpenPipe(const std::string& pipeName, const std::atomic<bool>& stop, int timeout = 0);

// NOTE: for simplicity we can use max buffer size ...
template<typename T = uint8_t>
std::vector<T> ReadFileAsync(HANDLE hFile, const std::atomic<bool>& stop) {
    DWORD dwBytesRead = 0;
    OVERLAPPED stOverlapped = { 0 };
    std::vector<T> tmpBuffer(1024 * 1024 * 2); // 2 MB

    if (!ReadFile(hFile, tmpBuffer.data(), tmpBuffer.size() * sizeof(T), &dwBytesRead, &stOverlapped)) {
        if (GetLastError() == ERROR_IO_PENDING) {
            while (!stop) {
                if (GetOverlappedResult(hFile, &stOverlapped, &dwBytesRead, FALSE)) {
                    // ReadFile operation completed ...
                    break;
                }
                else {
                    // pending ...
                    if (GetLastError() != ERROR_IO_INCOMPLETE) {
                        //Log("ReadFileAsync IO incomplete!");
                        throw GetLastErrorException("ReadFileAsync IO incomplete! "); // remote side was closed or smth else ...
                    }
                }
                Sleep(1); // TODO: for capturing frame use less delay ...
            }
        }
        else {
            throw GetLastErrorException("ReadFileAsync: "); // Read some error ...
        }
    }
    else {
        // ReadFile completed synchronously ...
    }

    if (stop || dwBytesRead == 0)
        return {};


    tmpBuffer.resize(dwBytesRead / sizeof(T));
    return std::move(tmpBuffer);
}


template <typename T = uint8_t>
void WriteToPipe(HANDLE hNamedPipe, const std::vector<T>& writeData) {
    DWORD cbWritten;
    if (!WriteFile(hNamedPipe, writeData.data(), writeData.size() * sizeof(T), &cbWritten, NULL)) {
        throw PipeError::WriteError;
    }
}

template <typename T = uint8_t>
std::vector<T> ReadFromPipeAsync(HANDLE hNamedPipe, const std::atomic<bool>& stop) {
    std::vector<T> tmp;
    try {
        tmp = ReadFileAsync<T>(hNamedPipe, stop);
    }
    catch (...) {
        throw PipeError::ReadError;
    }

    return std::move(tmp);
}




// EnumMsg must contain "Connect" msg and first "None" msg (fix in future)
// TODO: fix when interrupt connection -> not process "None" msg
// TODO: remove try .. catch if was not error in future
template<typename EnumMsg, typename T = uint8_t>
class Channel {
public:
    struct Reply {
        std::vector<T> readData;
        EnumMsg type;
    };
    using ReadFunc = std::function<Reply()>;
    using WriteFunc = std::function<void(std::vector<T>&&, EnumMsg)>;

    ReadFunc bindedReadFunc = std::bind(&Channel::Read, this);
    WriteFunc bindedWriteFunc = std::bind(&Channel::Write, this, std::placeholders::_1, std::placeholders::_2);

    Channel() = default;
    void Create(const std::string& pipeName, std::function<bool(ReadFunc, WriteFunc)> listenHandler, int timeout = 0, bool securityAccess = false) {
        // Make security attributes to connect admin & user pipe if need
        PSECURITY_DESCRIPTOR psd = NULL;
        BYTE  sd[SECURITY_DESCRIPTOR_MIN_LENGTH];
        psd = (PSECURITY_DESCRIPTOR)sd;
        InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(psd, TRUE, (PACL)NULL, FALSE);
        SECURITY_ATTRIBUTES sa = { sizeof(sa), psd, FALSE };

        hNamedPipe = CreateNamedPipeA(
            pipeName.c_str(),
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            512, 512, 5000, securityAccess ? &sa : NULL);

        if (hNamedPipe == INVALID_HANDLE_VALUE) {
            throw PipeError::InvalidHandle;
        }

        closeChannel = false;
        threadChannel = std::thread([this, listenHandler, timeout] {
            while (!closeChannel) {
                loggerHandler("Waiting for connect...");
                if (WaitConnectPipe(hNamedPipe, closeChannel, timeout)) {
                    loggerHandler("Connected. Waiting for command...");

                    connected = true;
                    stopSignal = false; // used for multi threads (like listener, sender)
                    threadConnect = std::thread([this] { connectHandler(); }); // Not block listening (TODO: replace on std::async)
                    Write({}, EnumMsg::Connect);
                    while (!stopSignal) {
                        if (listenHandler(bindedReadFunc, bindedWriteFunc) == false) {
                            stopSignal = true;
                        }
                    }
                    threadInterrupt = std::thread([this] {
                        interruptHandler();
                        }); // To avoid deadlock
                }
                else {
                    closeChannel = true;
                    stopSignal = true; // mb unnecessary?
                    loggerHandler("Timeout connection ...");
                }

                cvFinishSendingMessage.notify_all();
                connected = false; // if we here - connected == false
                StopSecondThreads();
                DisconnectNamedPipe(hNamedPipe);
            }
            });
    }


    void Open(const std::string& pipeName, std::function<bool(ReadFunc, WriteFunc)> listenHandler, int timeout = 10'000) {
        closeChannel = false;
        hNamedPipe = WaitOpenPipe(pipeName, closeChannel, timeout);

        if (hNamedPipe == INVALID_HANDLE_VALUE) {
            throw PipeError::InvalidHandle;
        }

        threadChannel = std::thread([this, listenHandler] {
            connected = true;
            stopSignal = false;
            threadConnect = std::thread([this] { connectHandler(); }); // Not block listening
            Write({}, EnumMsg::Connect);
            while (!stopSignal) {
                if (listenHandler(bindedReadFunc, bindedWriteFunc) == false) {
                    stopSignal = true;
                }
            }
            threadInterrupt = std::thread([this] {
                interruptHandler();
                }); // To avoid deadlock

            cvFinishSendingMessage.notify_all();
            connected = false; // if we here - connected == false
            });
    }


    void StopChannel() {
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
    ~Channel() {
        StopChannel();
        if (hNamedPipe)
            CloseHandle(hNamedPipe);
    }

    void SetLoggerHandler(std::function<void(std::string)> handler) {
        loggerHandler = handler;
    }

    void SetInterruptHandler(std::function<void()> handler) {
        interruptHandler = handler;
    }

    void SetConnectHandler(std::function<void()> handler) {
        connectHandler = handler;
    }



    void ClearPendingMessages() {
        pendingMessages.clear();
    }

    void WritePendingMessages() {
        try {
            for (auto& message : pendingMessages) {
                WriteToPipe<T>(hNamedPipe, message);
                if (isWaitingSendingMessage && static_cast<EnumMsg>(message.back()) == waitedMessage) { // TODO: test this case
                    cvFinishSendingMessage.notify_all();
                }
            }
            ClearPendingMessages();
        }
        catch (PipeError error) {
            switch (error)
            {
            case PipeError::WriteError:
                loggerHandler("Write error");
                break;
            };
            stopSignal = true;
        }
    }

    // Use rvalue ref [orig data will be lost]
    void Write(std::vector<T>&& writeData, EnumMsg type) {
        writeData.push_back(static_cast<T>(type));
        if (!connected) {
            pendingMessages.push_back(std::move(writeData));
            return;
        }

        try {
            WriteToPipe<T>(hNamedPipe, writeData);
            if (isWaitingSendingMessage && type == waitedMessage) {
                cvFinishSendingMessage.notify_all();
            }
        }
        catch (PipeError error) {
            switch (error)
            {
            case PipeError::WriteError:
                loggerHandler("Write error");
                break;
            };
            stopSignal = true;
        }
    }

    // TODO: add logic to wait for one of several messages
    void WaitFinishSendingMessage(EnumMsg type) {
        if (hNamedPipe == INVALID_HANDLE_VALUE)
            return;

        std::mutex m;
        std::unique_lock<std::mutex> lk(m);

        waitedMessage = type;
        isWaitingSendingMessage = true;
        cvFinishSendingMessage.wait(lk);
        isWaitingSendingMessage = false;
    }

private:
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
                loggerHandler("Read error");
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

    HANDLE hNamedPipe = nullptr;
    std::thread threadChannel;
    std::thread threadConnect;
    std::thread threadInterrupt;
    std::atomic<bool> connected = false;
    std::atomic<bool> stopSignal = false;
    std::atomic<bool> closeChannel = false;
    //std::function<void(std::string)> loggerHandler = [](std::string msg) {fprintf(stdout, "%s [%ld]\n", msg.c_str(), GetLastError()); };
    std::function<void(std::string)> loggerHandler = [](std::string msg) {};
    std::function<void()> interruptHandler = []() {};
    std::function<void()> connectHandler = []() {};
    std::vector<std::vector<T>> pendingMessages;

    EnumMsg waitedMessage = EnumMsg::None;
    std::atomic<bool> isWaitingSendingMessage = false;
    std::condition_variable cvFinishSendingMessage;
};