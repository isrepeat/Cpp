#pragma once
#include <string>

class PlatformProvider {
public:
	PlatformProvider();
	~PlatformProvider() = default;

	std::string GetOsName();
};