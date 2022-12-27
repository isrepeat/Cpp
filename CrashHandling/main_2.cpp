//#include "callstack.h"
//
//#define CALLSTACK_MAX_DEPTH 20
//
//void PrintStacktrace() {
//    LPVOID callStack[CALLSTACK_MAX_DEPTH];
//    DWORD  dwCallStackDepth = CallStack::getInstance().stackTrace(callStack, CALLSTACK_MAX_DEPTH);
//
//    for (DWORD j = 0; j < dwCallStackDepth; ++j)
//    {
//        CallStack::FuncInfo funcInfo;
//        CallStack::getInstance().getFuncInfo(callStack[j], funcInfo);
//        //printf("[0x%08lx] %s:%d {%s}\n", (DWORD)callStack[j], funcInfo.szFilePath, funcInfo.dwLineNumber, funcInfo.szFuncName);
//        printf("%s:%d {%s}\n", funcInfo.szFilePath, funcInfo.dwLineNumber, funcInfo.szFuncName);
//    }
//}
//
//void Foo(int x) {
//    printf("enter to foo \n");
//    PrintStacktrace();
//}
//
//int main()
//{
//    Foo(1);
//    Sleep(5000);
//    //system("pause");
//    return 0;
//}