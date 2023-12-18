#pragma once
#include "Platform.h"
#include <string>

struct UserAddress {
	std::string ip;
	std::uint16_t port;

	bool sameNetworkWithPartner = false;
};
