#pragma once
#include <string>
#include <Windows.h>
// #include <winnt.h> // EXCEPTION_POINTERS

#ifdef BUILD_LIBRARY
#define API __declspec(dllexport)
#else
//#define API // for static
#define API __declspec(dllimport) // for dll
#endif


namespace CrashAPI {
	API void CreateMinidump(EXCEPTION_POINTERS* pep, const wchar_t* path);
}