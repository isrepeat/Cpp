#pragma once
#include "common.h"
#include <string>

class IpTranslator {
public:
	static std::string Ipv4ToIpv6(const std::string& ipv4);
};

