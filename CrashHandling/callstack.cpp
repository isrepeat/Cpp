#define _CRT_SECURE_NO_WARNINGS
#include <string.h>

#include "algo.h"
#include "error.h"
#include "system.h"
#include "callstack.h"

#pragma comment (lib, "DbgHelp.lib")


//#ifdef _DEBUG

bool CallStack::loaded = false;

CallStack::CallStack()
{
#ifdef WIN32
    hProcess = GetCurrentProcess();
    DWORD dwOptions = SymGetOptions();
    dwOptions |= SYMOPT_LOAD_LINES;
    dwOptions |= SYMOPT_DEBUG;
    if (!loaded)
    {
        if (SymSetOptions(dwOptions) == 0) {
            throw QLanguage::Library::error<const char*>("SymSetOptions failed", __FILE__, __LINE__);
        }
        if (SymInitialize(GetCurrentProcess(), NULL, TRUE) == FALSE) {
            throw QLanguage::Library::error<const char*>("SymInitialize failed", __FILE__, __LINE__);
        }
        if (!loadAllModules()) {
            throw QLanguage::Library::error<const char*>("loadAllModules failed", __FILE__, __LINE__);
        }
        loaded = true;
    }
#endif
}

CallStack::~CallStack()
{
}

DWORD CallStack::stackTrace(LPVOID* pCallStack, DWORD dwMaxDepth)
{
#ifdef WIN32
    return CaptureStackBackTrace(0, dwMaxDepth, pCallStack, NULL);
#else
    return backtrace(pCallStack, dwMaxDepth);
#endif
}

bool CallStack::loadAllModules()
{
#ifdef WIN32
    DWORD dwNeeded = 0;
    if (!EnumProcessModules(hProcess, hModule, sizeof(hModule), &dwNeeded)) return false;

    const int iCount = dwNeeded / sizeof(HMODULE);

    for (int i = 0; i < iCount; ++i)
    {
        MODULEINFO info;

        GetModuleInformation(hProcess, hModule[i], &info, sizeof(info));
        GetModuleFileNameEx(hProcess, hModule[i], szImageName, iMax);
        GetModuleBaseName(hProcess, hModule[i], szModuleName, iMax);

#ifdef X64
        SymLoadModule64(hProcess, hModule[i], szImageName, szModuleName, (DWORD64)info.lpBaseOfDll, info.SizeOfImage);
#else
        SymLoadModule(hProcess, hModule[i], szImageName, szModuleName, (DWORD)info.lpBaseOfDll, info.SizeOfImage);
#endif
    }
#endif
    return true;
}

void CallStack::getFuncInfo(LPVOID dwFunc, FuncInfo& info)
{
    memset(szBuffer, 0, sizeof(szBuffer));
#ifdef WIN32
//#ifdef X64
    PIMAGEHLP_SYMBOL64 symbol = (PIMAGEHLP_SYMBOL64)szBuffer;
    symbol->SizeOfStruct = sizeof(szBuffer);
    symbol->MaxNameLength = sizeof(szBuffer) - sizeof(IMAGEHLP_SYMBOL64);

    DWORD64 dwDisplacement = 0;

    if (SymGetSymFromAddr64(hProcess, (DWORD)dwFunc, &dwDisplacement, symbol))
    {
        strncpy(info.szFuncName, symbol->Name, min(sizeof(info.szFuncName) - 1, strlen(symbol->Name)));
        info.szFuncName[min(sizeof(info.szFuncName) - 1, strlen(symbol->Name))] = 0;
    }

    IMAGEHLP_LINE64 imageHelpLine;
    imageHelpLine.SizeOfStruct = sizeof(imageHelpLine);

    if (SymGetLineFromAddr64(hProcess, (DWORD)dwFunc, (PDWORD)&dwDisplacement, &imageHelpLine))
    {
        strncpy(info.szFilePath, imageHelpLine.FileName, min(sizeof(info.szFilePath) - 1, strlen(imageHelpLine.FileName)));
        info.szFilePath[min(sizeof(info.szFilePath) - 1, strlen(imageHelpLine.FileName))] = 0;
        info.dwLineNumber = imageHelpLine.LineNumber;
    }
//#else
//    PIMAGEHLP_SYMBOL symbol = (PIMAGEHLP_SYMBOL)szBuffer;
//    symbol->SizeOfStruct = sizeof(szBuffer);
//    symbol->MaxNameLength = sizeof(szBuffer) - sizeof(IMAGEHLP_SYMBOL);
//
//
//    DWORD64 dwDisplacementSym = 0;
//    if (SymGetSymFromAddr(hProcess, (DWORD)dwFunc, &dwDisplacementSym, symbol))
//    {
//        strncpy(info.szFuncName, symbol->Name, min(sizeof(info.szFuncName) - 1, strlen(symbol->Name)));
//        info.szFuncName[min(sizeof(info.szFuncName) - 1, strlen(symbol->Name))] = 0;
//    }
//
//    IMAGEHLP_LINE imageHelpLine;
//    imageHelpLine.SizeOfStruct = sizeof(imageHelpLine);
//
//    DWORD dwDisplacementLine = 0;
//    if (SymGetLineFromAddr(hProcess, (DWORD)dwFunc, &dwDisplacementLine, &imageHelpLine))
//    {
//        strncpy(info.szFilePath, imageHelpLine.FileName, min(sizeof(info.szFilePath) - 1, strlen(imageHelpLine.FileName)));
//        info.szFilePath[min(sizeof(info.szFilePath) - 1, strlen(imageHelpLine.FileName))] = 0;
//        info.dwLineNumber = imageHelpLine.LineNumber;
//    }
//#endif
#endif
}

CallStack& CallStack::getInstance()
{
    static CallStack cs;
    return cs;
}

//#endif