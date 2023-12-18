#pragma once
#include "Platform.h"
#include "UserAddress.h"

struct UserAuthMessage {
	UserAddress localAddress;
	std::string jwt;
	std::uint64_t requestedUserServerId;
	std::uint64_t requesterUserServerId;
};