#pragma once
#include "Platform.h"
#include <string>
#include "UserAddress.h"

struct ActiveUser {
	UserAddress globalAddress;
	UserAddress localAddress;

	std::uint64_t idOnServer;
};