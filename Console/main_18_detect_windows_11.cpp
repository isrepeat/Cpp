#include <Windows.h>
#include <iostream>
#include <string>
#include "../Shared/ComAPI/ComAPI.h"

#pragma comment(lib, "wbemuuid.lib")
#pragma warning(disable : 4996)

int main() {
	OSVERSIONINFOA version;
	version.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	GetVersionExA(&version);

	const uint16_t major = version.dwMajorVersion == 10 && version.dwBuildNumber > 22'000 ? 11 : version.dwMajorVersion;


	wprintf(L"Windows %s \n", ComApi::WindowsVersion().c_str());
	wprintf(L"Windows %s %s \n", std::to_wstring(major).c_str(), std::to_wstring(version.dwBuildNumber).c_str());


	Sleep(10'000);
	return 0;
}
