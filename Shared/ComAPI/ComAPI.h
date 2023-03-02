#pragma once
#include <functional>
#include <windows.h>
#include <string>
#pragma comment(lib, "RuntimeObject.lib")

#ifdef BUILD_LIBRARY
#define API __declspec(dllexport)
#else
#define API // for static
//#define API __declspec(dllimport) // for dll
#endif

extern "C" {
	namespace ComApi {
		std::wstring API GetPackageFolder();
		std::wstring API WindowsVersion();
	}
}