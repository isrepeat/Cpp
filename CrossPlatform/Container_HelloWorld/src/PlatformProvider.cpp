#include "Platform.h"
#include "PlatformProvider.h"

PlatformProvider::PlatformProvider() {
}

std::string PlatformProvider::GetOsName() {
#if defined(__WINDOWS_OS__)
	return "Windows";
#elif defined(__UNIX_OS__)
	return "Linux";
#elif
	return "Unknown";
#endif
}