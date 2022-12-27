#pragma once

#include <windows.h>
#include <WinDNS.h>
#include <DbgHelp.h>
#include <Psapi.h>

#include "definition.h"


class CallStack
{
public:
    CallStack();
    ~CallStack();

    struct FuncInfo
    {
        char  szFuncName[512];
        char  szFilePath[1024];
        DWORD dwLineNumber;
    };

    DWORD stackTrace(LPVOID* pCallStack, DWORD dwMaxDepth);
    void getFuncInfo(LPVOID dwFunc, FuncInfo& info);

    static CallStack& getInstance();
protected:
    bool loadAllModules();

    static bool loaded;

    enum { maxBufferLength = 4096 };
    char szBuffer[maxBufferLength];
#ifdef WIN32
    HANDLE hProcess;
#endif
    enum { iMax = 4096 };
#ifdef WIN32
    HMODULE hModule[iMax];
#endif
    char szModuleName[iMax];
    char szImageName[iMax];
};