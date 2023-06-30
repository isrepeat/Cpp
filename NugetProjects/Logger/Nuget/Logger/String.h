#pragma once
#include <string>

// In order not to keep windows.h (and it deps headers) in header and avoid conflicts include only part of <winnls.h> 
#ifndef _WINNLS_
//
//  Code Page Default Values.
//  Please Use Unicode, either UTF-16 (as in WCHAR) or UTF-8 (code page CP_ACP)
//
#define CP_ACP                    0           // default to ANSI code page
#define CP_OEMCP                  1           // default to OEM  code page
#define CP_MACCP                  2           // default to MAC  code page
#define CP_THREAD_ACP             3           // current thread's ANSI code page
#define CP_SYMBOL                 42          // SYMBOL translations

#define CP_UTF7                   65000       // UTF-7 translation
#define CP_UTF8                   65001       // UTF-8 translation
#endif

std::string WStrToStr(const std::wstring& wstr, int codePage = CP_ACP);