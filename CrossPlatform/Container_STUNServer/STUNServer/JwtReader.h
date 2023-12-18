#pragma once
#include "Platform.h"
#include <string>
#include <jwt-cpp/include/jwt-cpp/jwt.h>

class JwtReader
{
public:
	JwtReader(std::string key, std::string issuer);
	bool IsValidToken(const std::string& token);
	uint32_t ReadUserId(const std::string& token);

private: 
	bool ValidateDecodedToken(const jwt::decoded_jwt<jwt::picojson_traits>& token);

private:
	
	jwt::verifier<jwt::default_clock, jwt::picojson_traits> verifier{ jwt::default_clock{} };


};

