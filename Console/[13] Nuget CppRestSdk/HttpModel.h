#pragma once
#include <MagicEnum/MagicEnum.h>

namespace Http {
    enum class ErrorCode;
};

// Add specialization for http errors because by default magic_enum range is [-128, 127]
// NOTE: need define before first usage magic_enum with Http::ErrorCode (for example ErrorCodeToString)
namespace magic_enum {
    namespace customize {
        template <>
        struct enum_range<Http::ErrorCode> {
            static_assert(std::is_enum_v<Http::ErrorCode>, "magic_enum::customize::enum_range requires enum type.");
            static constexpr int min = 0;
            static constexpr int max = 600;
            static_assert(max > min, "magic_enum::customize::enum_range requires max > min.");
        };
    }
}

namespace Http {
    enum class RequestType {
        Independent,
        Mixed, // It is assumed that such requests are chained, next requests must be handled after previous requests are successful.
    };
    enum class Method {
        GET,
        POST,
    };
    enum class AuthorizationScheme {
        Bearer,
        Amx
    };

    enum class ErrorCode : int {
        // https://powerusers.microsoft.com/t5/Building-Power-Apps/Server-return-a-HTTP-error-with-code-0/td-p/48491
        // can happen when losing connection after server started listening but before connection succeeded
        RequestNotCompleted = 0,

        BadRequest = 400,
        Unauthorized = 401,
        Forbidden = 403,
        RequestTimeout = 408,
    };

    inline std::string_view ErrorCodeToString(int statusCode) {
        return magic_enum::enum_name(static_cast<ErrorCode>(statusCode));
    }
}