#include "callstack.h"

#define CALLSTACK_MAX_DEPTH 10

void PrintStacktrace() {
    LPVOID callStack[CALLSTACK_MAX_DEPTH];
    DWORD  dwCallStackDepth = CallStack::getInstance().stackTrace(callStack, CALLSTACK_MAX_DEPTH);

    for (DWORD j = 0; j < dwCallStackDepth; ++j)
    {
        CallStack::FuncInfo funcInfo;
        CallStack::getInstance().getFuncInfo(callStack[j], funcInfo);
        printf("%s:%d {%s}\n", funcInfo.szFilePath, funcInfo.dwLineNumber, funcInfo.szFuncName);
    }
}

void Foo(int x) {
    PrintStacktrace();
}

int main()
{
 
    Foo(1);
    Sleep(1000);
    //system("pause");
    return 0;
}