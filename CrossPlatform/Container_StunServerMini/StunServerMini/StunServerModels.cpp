#include "StunServerModels.h"

namespace N {
    std::vector<uint8_t> SerializeCustomMessage(const std::string message) {
        return SerializeCustomMessage(std::vector<uint8_t>{message.begin(), message.end()});
    }

    std::vector<uint8_t> SerializeCustomMessage(const std::vector<uint8_t>& message) {
        std::vector<uint8_t> storage;
        storage.resize(sizeof(N::Server::CustomMessage) + message.size());

        N::Server::CustomMessage* customMessage = reinterpret_cast<N::Server::CustomMessage*>(storage.data());
        *customMessage = N::Server::CustomMessage{}; // copy default fields
        customMessage->msgSize = message.size();
        customMessage->msgData = storage.data() + sizeof(N::Server::CustomMessage);

        std::copy(message.begin(), message.end(), storage.data() + sizeof(N::Server::CustomMessage));
        return storage;
    }


    std::unique_ptr<std::string> DeserializeCustomMessageToString(const std::vector<uint8_t>& serializedCustomMessage) {
        auto itBegin = serializedCustomMessage.begin();
        auto itEndOfCustomMessage = serializedCustomMessage.begin() + sizeof(N::Server::CustomMessage);

        // detect size
        auto customMessage = N::Server::CustomMessage{};
        auto customMessageBytesPtr = reinterpret_cast<uint8_t*>(&customMessage);
        std::copy(itBegin, itEndOfCustomMessage, customMessageBytesPtr);

        // make uniq
        auto stringUniq = std::make_unique<std::string>(customMessage.msgSize, '\0');
        auto stringUniqDataPtr = reinterpret_cast<uint8_t*>(stringUniq.get()->data());

        // copy vector data to string
        std::copy(itEndOfCustomMessage, itEndOfCustomMessage + customMessage.msgSize, stringUniqDataPtr);
        return stringUniq;
    }

    std::unique_ptr<N::Server::CustomMessage> DeserializeCustomMessage(const std::vector<uint8_t>& serializedCustomMessage) {
        auto itBegin = serializedCustomMessage.begin();
        auto itEndOfCustomMessage = serializedCustomMessage.begin() + sizeof(N::Server::CustomMessage);

        // detect size
        auto customMessage = N::Server::CustomMessage{};
        auto customMessageBytesPtr = reinterpret_cast<uint8_t*>(&customMessage);
        std::copy(itBegin, itEndOfCustomMessage, customMessageBytesPtr);

        // make uniq
        auto customMessageUniq = std::make_unique<N::Server::CustomMessage>(customMessage);
        auto customMessageUniqBytesPtr = reinterpret_cast<uint8_t*>(customMessageUniq.get());

        // correct msgData ptr and copy data
        customMessageUniq->msgData = customMessageUniqBytesPtr + sizeof(N::Server::CustomMessage);
        std::copy(itEndOfCustomMessage, itEndOfCustomMessage + customMessageUniq->msgSize, customMessageUniq->msgData);
        return customMessageUniq;
    }
}