#pragma once
#include "common.h"
#ifdef BOOST_SUPPORTED
#include <string>

namespace HELPERS_NS {
	std::string GetLocalIp(int attempts = 1);
}
#endif