#pragma once
#if defined(_WIN32) || defined(_WIN64)
#define __WINDOWS_OS__
#elif defined(__unix__) || defined(__unix)  || defined(unix)
#define __UNIX_OS__
#endif