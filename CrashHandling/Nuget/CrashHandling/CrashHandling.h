#pragma once
#include <string>
#include <functional>
#include <Windows.h>

#ifdef __MAKE_DLL__
#define API __declspec(dllexport)
#else
//#define API // for static
#define API __declspec(dllimport) // for dll
#endif

// Need comile with /EHa
namespace CrashHandling {
	API void RegisterVectorHandler(PVECTORED_EXCEPTION_HANDLER handler);
	API void OpenMinidumpChannel(EXCEPTION_POINTERS* pep, std::wstring packageFolder, std::wstring channelName = L"\\\\.\\pipe\\Local\\channelDumpWriter");
}