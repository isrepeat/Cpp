#pragma once
#include "Platform.h"
#include <cinttypes>

namespace N {
    enum class QueryType {
        None,
        GetMyAddress,
        GetPartnerAddress,
    };

    enum class ResponseType {
        None,
        UserAddress,
    };

#pragma pack(push, 1)
    template<QueryType queryType>
    struct Query {
        const QueryType type = queryType;
    };

    template<ResponseType responseType>
    struct Response {
        const ResponseType type = responseType;
    };

    // Helpers:
    struct endpoint {
        uint32_t ip = 0;
        uint16_t port = 0;
    };


    // Client queries
    namespace Client {
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
    }
#pragma pack(pop)
}
