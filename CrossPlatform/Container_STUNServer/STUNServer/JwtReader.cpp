#include "JwtReader.h"
#include "Logger.h"

namespace {
    const std::string userIdClaims = "user_id";
}

JwtReader::JwtReader(std::string key, std::string issuer) {
    verifier.allow_algorithm(jwt::algorithm::hs384{ key }).with_issuer(issuer);
}

bool JwtReader::IsValidToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        return ValidateDecodedToken(decoded);
    }
    catch (...) {
        //WriteError(("Token validation failed, token value: " + token).c_str());
        //printf("Token validation failed, token value: \"%s\"\n", token.c_str());
        return false;
    }
}

uint32_t JwtReader::ReadUserId(const std::string& token) {
    auto decoded = jwt::decode(token);
    auto idClaim = decoded.get_payload_claim(userIdClaims);
   
    return idClaim.as_int();
}

bool JwtReader::ValidateDecodedToken(const jwt::decoded_jwt<jwt::picojson_traits>& token) {
    auto error = std::error_code{};
    verifier.verify(token, error);
    return !(bool)error;
}
