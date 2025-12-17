#pragma once
#include "common.h"
#if COMPILE_FOR_DESKTOP
#include "HWindows.h"
#include <MagicEnum/MagicEnum.h>

#include "ConcurrentQueue.h"
#include "LocalPtr.hpp"
#include "Helpers.h"
#include "Logger.h"
#include "Thread.h"
#include "File.h"
#include "Time.h"

#include <condition_variable>
#include <functional>
#include <string>
#include <vector>
#include <thread>
#include <sddl.h>
#include <queue>

namespace HELPERS_NS {
#define BUFFER_PIPE READ_FILE_BUFFER_SIZE_DEFAULT

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


    template <typename T = uint8_t>
    void ReadFromPipeAsync(HANDLE hNamedPipe, const std::atomic<bool>& stop, std::vector<T>& outBuffer) {
        try {
            HELPERS_NS::ReadFileAsync<T>(hNamedPipe, stop, outBuffer, BUFFER_PIPE);
        }
        catch (const std::exception& ex) {
            LOG_ERROR_D("Catch ReadFileAsync exception = {}", ex.what());
            throw PipeError::ReadError;
        }
    }

    template <typename T = uint8_t>
    void WriteToPipeAsync(HANDLE hNamedPipe, const std::atomic<bool>& stop, std::span<T> writeData) {
        if (writeData.empty())
            return;

        try {
            HELPERS_NS::WriteFileAsync<T>(hNamedPipe, stop, writeData);
        }
        catch (const std::exception& ex) {
            LOG_ERROR_D("Catch WriteFileAsync exception = {}", ex.what());
            throw PipeError::WriteError;
        }
    }


    template<typename T>
    HELPERS_NS::LocalPtr<T> GetTokenInfo(HANDLE hToken, TOKEN_INFORMATION_CLASS typeInfo) {
        DWORD dwSize = 0;
        if (!GetTokenInformation(hToken, typeInfo, NULL, 0, &dwSize) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return nullptr;
        }

        if (auto pTokenData = HELPERS_NS::LocalPtr<T>(LocalAlloc(LPTR, dwSize))) {
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
    class Channel : public HELPERS_NS::IThread {
        CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(Channel);

    public:
#pragma pack(push, 1)
        struct MessageDescriptor {
            uint32_t size = 0;
            uint32_t type = 0;
        };
#pragma pack(pop)

        struct Message {
            EnumMsg type;
            std::vector<T> payload;
        };

        struct MessageInternal {
            MessageDescriptor descriptor;
            Message message;
            uint32_t processedBytes = 0;
        };


        using Msg_t = std::shared_ptr<Message>;
        using WriteFunc = std::function<void(EnumMsg, std::vector<T>&&)>;

        Channel()
            : messagesQueue{ std::make_shared<HELPERS_NS::ConcurrentQueue<Msg_t>>() }
        {
            LOG_FUNCTION_ENTER("Channel()");
            this->readStreamBuffer.reserve(BUFFER_PIPE * 2); // reserve double size
        }

        ~Channel() {
            LOG_FUNCTION_ENTER_C("~Channel()");
            this->StopChannel();
        }

        // Make security attributes to connect admin & user pipe
        void CreateForAdmin(const std::wstring& pipeName, std::function<bool(Msg_t, WriteFunc)> listenHandler, int timeout = 0) {
            LOG_FUNCTION_ENTER(L"CreateForAdmin(pipeName = {}, ...)", pipeName);

            auto pSecurityDescriptor = std::make_unique<SECURITY_DESCRIPTOR>();
            InitializeSecurityDescriptor(pSecurityDescriptor.get(), SECURITY_DESCRIPTOR_REVISION);
            SetSecurityDescriptorDacl(pSecurityDescriptor.get(), TRUE, (PACL)NULL, FALSE);
            this->pSecurityAttributes = std::make_unique<SECURITY_ATTRIBUTES>(SECURITY_ATTRIBUTES{ sizeof(SECURITY_ATTRIBUTES), pSecurityDescriptor.get(), FALSE });

            this->Create(pipeName, listenHandler, timeout);
        }

        // Make security attributes to connect UWP & desktop apps
        void CreateForUWP(const std::wstring& pipeName, std::function<bool(Msg_t, WriteFunc)> listenHandler, HANDLE hProcessUWP, int timeout = 0) {
            LOG_FUNCTION_ENTER(L"CreateForUWP(pipeName = {}, ...)", pipeName);

            this->pSecurityAttributes = std::make_unique<SECURITY_ATTRIBUTES>(SECURITY_ATTRIBUTES{ sizeof(SECURITY_ATTRIBUTES), NULL, FALSE });

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
                        HELPERS_NS::LocalPtr<WCHAR> pStr;
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

            this->Create(formattedPipeName, listenHandler, timeout);
        }

        void Create(const std::wstring& pipeName, std::function<bool(Msg_t, WriteFunc)> listenHandler, int timeout = 0) {
            LOG_FUNCTION_ENTER(L"Create(pipeName = {}, listenHandler, timeout = {})", pipeName, timeout);
            this->pipeName = pipeName;

            this->hNamedPipe = CreateNamedPipeW(
                pipeName.c_str(),
                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                BUFFER_PIPE * sizeof(T), BUFFER_PIPE * sizeof(T), 5000, pSecurityAttributes.get());

            if (this->hNamedPipe == INVALID_HANDLE_VALUE) {
                throw PipeError::InvalidHandle;
            }

            this->closeChannel = false;
            this->threadChannel = std::thread([this, listenHandler, timeout, pipeName] {
                LOG_THREAD(this->pipeName + L" threadChannel Create");

                while (!this->closeChannel) {
                    LOG_DEBUG("Waiting for connect...");
                    auto status = WaitConnectPipe(this->hNamedPipe, this->closeChannel, timeout);
                    LOG_DEBUG("WaitConnectPipe status = {}", magic_enum::enum_name(status));

                    switch (status) {
                    case PipeConnectionStatus::Connected: {
                        this->ListenRoutine(listenHandler);
                        break;
                    }
                    case PipeConnectionStatus::Error:
                    case PipeConnectionStatus::Stopped:
                    case PipeConnectionStatus::TimeoutConnection:
                        this->closeChannel = true;
                        this->stopSignal = true;
                        break;
                    }

                    this->StopSecondThreads();
                    DisconnectNamedPipe(hNamedPipe);
                }
                });
        }

        void Open(const std::wstring& pipeName, std::function<bool(Msg_t, WriteFunc)> listenHandler, int timeout = 10'000) {
            LOG_FUNCTION_ENTER(L"Open(pipeName = {}, listenHandler, timeout = {})", pipeName, timeout);
            this->pipeName = pipeName;

            this->closeChannel = false;
            auto status = WaitOpenPipe(this->hNamedPipe, pipeName, this->closeChannel, timeout);
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

            this->threadChannel = std::thread([this, listenHandler] {
                LOG_THREAD(this->pipeName + L" threadChannel Open");
                this->ListenRoutine(listenHandler);
                });
        }

        /* ------------------------------------ */
        /*			 IThread methods			*/
        /* ------------------------------------ */
        void NotifyAboutStop() override {
            LOG_FUNCTION_ENTER_C("NotifyAboutStop()");

            this->StopListening();
            this->closeChannel = true;
        }

        void WaitingFinishThreads() override {
            LOG_FUNCTION_ENTER_C("WaitingFinishThreads()");

            if (this->threadChannel.joinable()) // wait finish channel thread where init interrupt thread 
                this->threadChannel.join();

            this->StopSecondThreads();

            if (this->hNamedPipe) {
                CloseHandle(this->hNamedPipe);
                this->hNamedPipe = nullptr;
            }
        }

        bool IsConnected() {
            return this->connected;
        }

        void SetInterruptHandler(std::function<void()> handler) {
            this->interruptHandler = handler;
        }

        void SetConnectHandler(std::function<void()> handler) {
            this->connectHandler = handler;
        }

        void StopChannel() {
            LOG_FUNCTION_ENTER_C("StopChannel()");
            this->NotifyAboutStop();
            this->WaitingFinishThreads();
        }

        void Write(EnumMsg type, std::vector<T>&& writeData = {}) {
            std::unique_lock lk{ mxWrite };
            Message message{ type, std::move(writeData) };

            if (!this->connected) {
                this->pendingMessages.push_back(std::move(message));
                return;
            }

            this->WriteInternal(message);
        }

        void WritePendingMessages() {
            LOG_FUNCTION_ENTER_C("WritePendingMessages()");
            std::lock_guard lk{ mxWrite };
            for (auto& message : this->pendingMessages) {
                if (!this->WriteInternal(message))
                    break;
            }
            this->pendingMessages.clear();
        }

        void ClearPendingMessages() {
            LOG_FUNCTION_ENTER_C("ClearPendingMessages()");
            std::lock_guard lk{ mxWrite };
            this->pendingMessages.clear();
        }

        // TODO: add logic to wait for one of several messages
        template <typename DurationT = std::chrono::seconds>
        void WaitFinishSendingMessage(EnumMsg type, DurationT waitTimeout = DurationT{30}) {
            LOG_FUNCTION_ENTER_C("WaitFinishSendingMessage({}[{}], {})"
                , MagicEnum::ToString(type), static_cast<int>(type) // also log 'type' casted to int because ToString may return empty str if 'type' out of range.
                , waitTimeout
            );
            std::unique_lock lk{ mxWrite };

            if (this->hNamedPipe == INVALID_HANDLE_VALUE) {
                LOG_ERROR_D("hNamedPipe == INVALID_HANDLE_VALUE");
                return;
            }

            this->waitedMessage = type;
            this->cvFinishSendingMessage.wait_for(lk, waitTimeout); // now mutex unlocked and thread begin wait
            this->waitedMessage = EnumMsg::None;
            LOG_DEBUG_D("waiting is finished");
        }

    private:
        void ListenRoutine(std::function<bool(Msg_t, WriteFunc)> listenHandler) {
            LOG_FUNCTION_ENTER_C("ListenRoutine(listenHandler)");

            this->connected = true;
            this->stopSignal = false;
            this->messagesQueue->StartWork();

            this->threadConnect = std::thread([this] { // Not block current thread to avoid deadlock (TODO: replace on std::async)
                LOG_THREAD(this->pipeName + L" threadConnect");
                this->connectHandler();
                });

            this->Write(EnumMsg::Connect);

            this->threadRead = std::thread([this] {
                LOG_THREAD(this->pipeName + L" threadRead");
                this->ReadRoutine();
                });

            while (!this->stopSignal) {
                if (this->messagesQueue->IsWorking()) {
                    if (auto msg = this->messagesQueue->Pop()) {
                        if (listenHandler(msg, this->bindedWriteFunc) == false) {
                            this->StopListening();
                            LOG_DEBUG_D("listenHandler returned 'false', stop listening.");
                        }
                    }
                }
            }
            LOG_DEBUG_D("Finish ListenRoutine");

            this->threadInterrupt = std::thread([this] { // Not block current thread to avoid deadlock
                LOG_THREAD(this->pipeName + L" threadInterrupt");
                this->interruptHandler();
                });

            LOG_DEBUG_D("Notify cvFinishSendingMessage");
            this->cvFinishSendingMessage.notify_all();
            this->connected = false; // if we here pipe not connected
        }

        void ReadRoutine() {
            LOG_FUNCTION_ENTER_C("ReadRoutine()");
            MessageInternal messageInternal;

            try {
                while (!this->stopSignal) {
                    bool clearStreamBuffer = true;
                    ReadFromPipeAsync<T>(this->hNamedPipe, this->stopSignal, this->readStreamBuffer);

                    uint32_t processedBytes = 0;

                    // TODO: Write tests for cases when 'while' really need (mb remove?) 
                    while (processedBytes < this->readStreamBuffer.size()) {
                        if (!this->ParseMessage(this->readStreamBuffer, processedBytes, messageInternal)) {
                            clearStreamBuffer = false;
                            break; // need more data to read descriptor (keep bytes in readStreamBuffer)
                        }
                        if (messageInternal.processedBytes == sizeof(MessageDescriptor) + messageInternal.descriptor.size) { // if message completed
                            this->messagesQueue->Push(std::make_shared<Message>(Message{
                                .type = static_cast<EnumMsg>(messageInternal.descriptor.type),
                                .payload = std::move(messageInternal.message.payload)
                                }));

                            messageInternal = {};
                        }
                    }

                    if (clearStreamBuffer) {
                        // readStreamBuffer does not contain part of the next message
                        // (even if it resized inside ReadFromPipeAsync), so cleansing is safe. 
                        LOG_ASSERT(processedBytes == this->readStreamBuffer.size());
                        this->readStreamBuffer.clear(); // can clear buffer because payload was saved/moved in messagesQueue
                    }
                }
            }
            catch (PipeError error) {
                LOG_ERROR_D("Catch PipeError = {}", magic_enum::enum_name(error));

                switch (error) {
                case PipeError::ReadError:
                    LOG_ERROR_D("Read error! Stop channel.");
                    break;
                };
                this->StopListening();
            }
        }

        bool ParseMessage(std::vector<T>& readStreamBuffer, uint32_t& totalProcessedBytes, MessageInternal& messageInternal) {
            bool descriptorWasRead = messageInternal.processedBytes >= sizeof(MessageDescriptor);
            bool payloadWasRead = descriptorWasRead && messageInternal.processedBytes == sizeof(MessageDescriptor) + messageInternal.descriptor.size;

            if (!descriptorWasRead) {
                auto restStreamBytes = readStreamBuffer.size() - totalProcessedBytes;
                if (restStreamBytes < sizeof(MessageDescriptor)) {
                    readStreamBuffer.erase(readStreamBuffer.begin(), readStreamBuffer.begin() + totalProcessedBytes);
                    totalProcessedBytes = readStreamBuffer.size();
                    assert(restStreamBytes == totalProcessedBytes);
                    return false;
                }

                messageInternal.descriptor = *reinterpret_cast<MessageDescriptor*>(readStreamBuffer.data() + totalProcessedBytes);
                messageInternal.processedBytes += sizeof(MessageDescriptor);
                totalProcessedBytes += sizeof(MessageDescriptor);
            }

            if (!payloadWasRead) {
                if (messageInternal.descriptor.size > 0) {
                    auto remainingPayloadBytes = messageInternal.descriptor.size - messageInternal.message.payload.size();
                    auto restStreamBytes = readStreamBuffer.size() - totalProcessedBytes;
                    auto startIt = readStreamBuffer.begin() + totalProcessedBytes;

                    auto readPayloadBytes = restStreamBytes < remainingPayloadBytes ? restStreamBytes : remainingPayloadBytes;

                    messageInternal.message.payload.insert(messageInternal.message.payload.end(), startIt, startIt + readPayloadBytes);
                    messageInternal.processedBytes += readPayloadBytes;
                    totalProcessedBytes += readPayloadBytes;
                }
            }

            return true;
        }

        bool WriteInternal(Message& message) {
            try {
                MessageDescriptor msgDecriptor{ message.payload.size(), static_cast<uint8_t>(message.type) };
                T messageSizeBuffer[sizeof(uint32_t)];
                T messageTypeBuffer[sizeof(uint32_t)];
                std::memcpy(&messageSizeBuffer, &msgDecriptor.size, sizeof(uint32_t));
                std::memcpy(&messageTypeBuffer, &msgDecriptor.type, sizeof(uint32_t));

                WriteToPipeAsync<T>(hNamedPipe, stopSignal, messageSizeBuffer);
                WriteToPipeAsync<T>(hNamedPipe, stopSignal, messageTypeBuffer);
                WriteToPipeAsync<T>(hNamedPipe, stopSignal, message.payload);

                if (this->waitedMessage != EnumMsg::None && this->waitedMessage == message.type) {
                    LOG_DEBUG_D("Notify cvFinishSendingMessage");
                    this->cvFinishSendingMessage.notify_all();
                }

                return true;
            }
            catch (PipeError error) {
                LOG_ERROR_D("Catch PipeError = {}", magic_enum::enum_name(error));

                switch (error) {
                case PipeError::WriteError:
                    LOG_ERROR_D("Write error! Stop channel.");
                    break;
                };

                this->StopListening();
                return false;
            }
        }

        void StopListening() {
            LOG_FUNCTION_ENTER_C("StopListening()");
            this->stopSignal = true;
            this->messagesQueue->StopWork();
        }

        void StopSecondThreads() {
            if (this->threadConnect.joinable())
                this->threadConnect.join();

            if (this->threadRead.joinable())
                this->threadRead.join();

            if (this->threadInterrupt.joinable())
                this->threadInterrupt.join();
        }


    private:
        std::mutex mxWrite;
        HANDLE hNamedPipe = nullptr;
        std::thread threadRead;
        std::thread threadChannel;
        std::thread threadConnect;
        std::thread threadInterrupt;
        std::wstring pipeName; // used just for logging
        std::atomic<bool> connected = false;
        std::atomic<bool> stopSignal = false;
        std::atomic<bool> closeChannel = false;
        std::function<void()> connectHandler = []() {};
        std::function<void()> interruptHandler = []() {};
        std::vector<Message> pendingMessages;
        std::vector<T> readStreamBuffer;

        std::atomic<EnumMsg> waitedMessage = EnumMsg::None;
        std::condition_variable cvFinishSendingMessage;
        std::unique_ptr<SECURITY_ATTRIBUTES> pSecurityAttributes;

        const WriteFunc bindedWriteFunc = std::bind(&Channel::Write, this, std::placeholders::_1, std::placeholders::_2);

        std::shared_ptr<HELPERS_NS::ConcurrentQueue<Msg_t>> messagesQueue;
    };
}
#endif