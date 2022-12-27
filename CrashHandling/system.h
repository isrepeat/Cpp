#pragma once
#include "definition.h"

inline bool isX86()
{
    return sizeof(void*) == 4;
}

inline bool isX64()
{
    return sizeof(void*) == 8;
}

inline bool isWindows()
{
#ifdef WIN32
    return true;
#else
    return false;
#endif
}

inline bool isUnix()
{
#ifdef unix
    return true;
#else
    return false;
#endif
}

#define WINDOWS 0
#define UNIX    1

enum
{
    optr_type = (sizeof(void*) == 4) ? 86 : 64,
#ifdef WIN32
    os_type = WINDOWS,
#else
    os_type = UNIX,
#endif
};
