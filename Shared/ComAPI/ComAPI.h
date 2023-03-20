#pragma once
#include <functional>
#include <windows.h>
#include <string>
#pragma comment(lib, "RuntimeObject.lib")

extern "C" {
	namespace ComApi {
		std::wstring GetPackageFolder();
		std::wstring WindowsVersion();
	}
}