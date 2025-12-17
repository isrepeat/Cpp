#pragma once
/**
	It is a wrapper under HelpersWinApi nuget API. 
	You can include this project in "Universal window" project safety (nuget dependencies copied automatially).
*/
#include <Helpers\common.h>

#ifdef __MAKE_DYNAMIC_LIBRARY__
#define HELPERS_WINAPI_CX_API __declspec(dllexport) // used within this project
#else
#define HELPERS_WINAPI_CX_API __declspec(dllimport) // if nuget builds as dll redefine
#endif

namespace HELPERS_NS {
	namespace Cx {
		namespace WinApi {
			HELPERS_WINAPI_CX_API HWND GetDesktopWindow();
			HELPERS_WINAPI_CX_API bool GetWindowRect(HWND hWnd, LPRECT lpRect);
		}
	}
}