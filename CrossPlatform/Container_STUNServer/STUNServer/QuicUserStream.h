#pragma once
#include "Platform.h"
#include <msquic.h>
#include <string>
#include "UserAddress.h"

struct QuicUserStream {
	HQUIC stream;
	UserAddress userAddress;
};