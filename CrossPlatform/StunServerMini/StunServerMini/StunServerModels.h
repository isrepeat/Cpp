#pragma once
#include "Platform.h"
#include <cinttypes>

namespace N {
    enum class QueryType {
        None,
        Hello,
        GetMyAddress,
        GetPartnerAddress,
    };

    enum class ResponseType {
        None,
        UserAddress,
        CustomMessage,
    };

#pragma pack(push, 1)
    template<QueryType queryType>
    struct Query {
        /*const*/ QueryType type = queryType; // do not use const so that the operator= is not removed implicitly
    };

    template<ResponseType responseType>
    struct Response {
        /*const*/ ResponseType type = responseType;
    };

    // Helpers:
    struct endpoint {
        uint32_t ip = 0;
        uint16_t port = 0;
    };


    // Client queries
    namespace Client {
        struct Hello : Query<QueryType::Hello> {
        };
        struct Handshake : Query<QueryType::GetMyAddress> {
            endpoint localAddress;
        };
        struct AuthMessage : Query<QueryType::GetPartnerAddress> {
            endpoint localAddress;
            int localId = 0;
            int parnterId = 0;
        };
    }

    // Server responses
    namespace Server {
        struct UserAddress : Response<ResponseType::UserAddress> {
            endpoint local;
            endpoint global;
        };
        struct CustomMessage : Response<ResponseType::CustomMessage> {
            int msgSize;
            uint8_t* msgData;
        };
    }
#pragma pack(pop)

    std::vector<uint8_t> SerializeCustomMessage(const std::string message);
    std::vector<uint8_t> SerializeCustomMessage(const std::vector<uint8_t>& message);

    std::unique_ptr<std::string> DeserializeCustomMessageToString(const std::vector<uint8_t>& serializedCustomMessage);
    std::unique_ptr<N::Server::CustomMessage> DeserializeCustomMessage(const std::vector<uint8_t>& serializedCustomMessage);
}