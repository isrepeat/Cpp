//#define _CRT_SECURE_NO_WARNINGS
//#include <Windows.h>
//#include <iostream>
//#include <fstream>
//#include <stdio.h>
//#include <stdlib.h>
//#include <csignal>
//#include <csignal>
//#include <thread>
//#include <chrono>
//#include <vector>
//#include <string>
//
//#include <signal.h>
//#include <assert.h>
//#include <stdbool.h>
//#include <stdint.h>
//#include <stdbool.h>
//#include <errno.h>
////#include <imagehlp.h>
//
//#include "..\Helpers\Helpers.h"
//#include <dbghelp.h>
//#pragma comment (lib, "DbgHelp.lib")
//
//
//void SignalHandler(int signal) {
//    printf("SignalHandler \n");
//    std::string strSignal;
//    switch (signal) {
//    case SIGABRT:
//        strSignal = "SIGABRT";
//        break;
//    case SIGFPE:
//        strSignal = "SIGFPE";
//        break;
//    case SIGILL:
//        strSignal = "SIGILL";
//        break;
//    case SIGINT:
//        strSignal = "SIGINT";
//        break;
//    case SIGSEGV:
//        strSignal = "SIGSEGV";
//        break;
//    case SIGTERM:
//        strSignal = "SIGTERM";
//        break;
//    }
//
//    Sleep(1000);
//    int xxx = 9;
//}
//
//
//
//
//static const std::string exePath = H::ExeFullnameA();
//
///* Resolve symbol name and source location given the path to the executable
//   and an address */
//int addr2line(char const* const program_name, void const* const addr)
//{
//    char addr2line_cmd[512] = { 0 };
//
//    /* have addr2line map the address to the relent line in the code */
//#ifdef __APPLE__
//  /* apple does things differently... */
//    sprintf(addr2line_cmd, "atos -o %.256s %p", program_name, addr);
//#else
//    sprintf(addr2line_cmd, "addr2line -f -p -e %.256s %p", program_name, addr);
//    printf("addr2line -f -p -e %.256s %p", program_name, addr);
//#endif
//
//    return system(addr2line_cmd);
//}
//
//bool PrintAdrToLine(const char* modulePath, DWORD64 offset) {
//    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
//    HANDLE hProcess = GetCurrentProcess();
//    if (!SymInitialize(hProcess, NULL, FALSE)) {
//        printf("SymInitialize returned error : %u\n", GetLastError());
//        return FALSE;
//    }
//    DWORD64 ModBase = SymLoadModuleEx(hProcess, NULL, modulePath, NULL, 0, 0, NULL, 0);
//    if ((ModBase == 0) && (GetLastError() != ERROR_SUCCESS)) {
//        printf("SymLoadModuleEx returned error : %u\n", GetLastError());
//        return FALSE;
//    }
//    DWORD64 SymAddr = ModBase + offset;
//    printf("Module %s Loaded At 0x%I64x SymAddr = 0x%I64x\n", modulePath, ModBase, SymAddr);
//    IMAGEHLP_LINE64 line = { 0 };
//    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
//    DWORD dwDisplacement = 0;
//    if (!SymGetLineFromAddr64(hProcess, SymAddr, &dwDisplacement, &line)) {
//        printf("SymGetLineFromAddr64 returned error : %u\n", GetLastError());
//        return FALSE;
//    }
//    printf("Source= %s # %u @ %I64x\n", line.FileName, line.LineNumber, line.Address);
//    int xxx = 9;
//    //std::ifstream fs;
//    //fs.open(line.FileName);
//    //if (fs.is_open()) {
//    //    std::string buff;
//    //    unsigned  int lineno = 0;
//    //    while (lineno != line.LineNumber && getline(fs, buff)) {
//    //        ++lineno;
//    //    }
//    //    printf("%s\n", buff.c_str());
//    //    fs.close();
//    //}
//    //else {
//    //    printf("cannot open %s\n", line.FileName);
//    //}
//}
//
//
//
//
//
//
//
//void windows_print_stacktrace(CONTEXT* context)
//{
//    SymInitialize(GetCurrentProcess(), 0, true);
//
//    STACKFRAME frame = { 0 };
//
//    /* setup initial stack frame */
//    frame.AddrPC.Offset = context->Rip;
//    frame.AddrPC.Mode = AddrModeFlat;
//    frame.AddrStack.Offset = context->Rsp;
//    frame.AddrStack.Mode = AddrModeFlat;
//    frame.AddrFrame.Offset = context->Rbp;
//    frame.AddrFrame.Mode = AddrModeFlat;
//
//    while (StackWalk(IMAGE_FILE_MACHINE_I386,
//        GetCurrentProcess(),
//        GetCurrentThread(),
//        &frame,
//        context,
//        0,
//        SymFunctionTableAccess,
//        SymGetModuleBase,
//        0))
//    {
//        //addr2line(icky_global_program_name, (void*)frame.AddrPC.Offset);
//        PrintAdrToLine(exePath.c_str(), frame.AddrPC.Offset);
//    }
//
//    SymCleanup(GetCurrentProcess());
//}
//
//LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS* ExceptionInfo)
//{
//    std::cout << "Hello\n";
//    Sleep(1000);
//    switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
//    {
//    case EXCEPTION_ACCESS_VIOLATION:
//        fputs("Error: EXCEPTION_ACCESS_VIOLATION\n", stdout);
//        break;
//    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
//        fputs("Error: EXCEPTION_ARRAY_BOUNDS_EXCEEDED\n", stderr);
//        break;
//    case EXCEPTION_BREAKPOINT:
//        fputs("Error: EXCEPTION_BREAKPOINT\n", stderr);
//        break;
//    case EXCEPTION_DATATYPE_MISALIGNMENT:
//        fputs("Error: EXCEPTION_DATATYPE_MISALIGNMENT\n", stderr);
//        break;
//    case EXCEPTION_FLT_DENORMAL_OPERAND:
//        fputs("Error: EXCEPTION_FLT_DENORMAL_OPERAND\n", stderr);
//        break;
//    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
//        fputs("Error: EXCEPTION_FLT_DIVIDE_BY_ZERO\n", stdout);
//        break;
//    case EXCEPTION_FLT_INEXACT_RESULT:
//        fputs("Error: EXCEPTION_FLT_INEXACT_RESULT\n", stderr);
//        break;
//    case EXCEPTION_FLT_INVALID_OPERATION:
//        fputs("Error: EXCEPTION_FLT_INVALID_OPERATION\n", stderr);
//        break;
//    case EXCEPTION_FLT_OVERFLOW:
//        fputs("Error: EXCEPTION_FLT_OVERFLOW\n", stderr);
//        break;
//    case EXCEPTION_FLT_STACK_CHECK:
//        fputs("Error: EXCEPTION_FLT_STACK_CHECK\n", stderr);
//        break;
//    case EXCEPTION_FLT_UNDERFLOW:
//        fputs("Error: EXCEPTION_FLT_UNDERFLOW\n", stderr);
//        break;
//    case EXCEPTION_ILLEGAL_INSTRUCTION:
//        fputs("Error: EXCEPTION_ILLEGAL_INSTRUCTION\n", stderr);
//        break;
//    case EXCEPTION_IN_PAGE_ERROR:
//        fputs("Error: EXCEPTION_IN_PAGE_ERROR\n", stderr);
//        break;
//    case EXCEPTION_INT_DIVIDE_BY_ZERO:
//        fputs("Error: EXCEPTION_INT_DIVIDE_BY_ZERO\n", stderr);
//        break;
//    case EXCEPTION_INT_OVERFLOW:
//        fputs("Error: EXCEPTION_INT_OVERFLOW\n", stderr);
//        break;
//    case EXCEPTION_INVALID_DISPOSITION:
//        fputs("Error: EXCEPTION_INVALID_DISPOSITION\n", stderr);
//        break;
//    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
//        fputs("Error: EXCEPTION_NONCONTINUABLE_EXCEPTION\n", stderr);
//        break;
//    case EXCEPTION_PRIV_INSTRUCTION:
//        fputs("Error: EXCEPTION_PRIV_INSTRUCTION\n", stderr);
//        break;
//    case EXCEPTION_SINGLE_STEP:
//        fputs("Error: EXCEPTION_SINGLE_STEP\n", stderr);
//        break;
//    case EXCEPTION_STACK_OVERFLOW:
//        fputs("Error: EXCEPTION_STACK_OVERFLOW\n", stderr);
//        break;
//    default:
//        fputs("Error: Unrecognized Exception\n", stderr);
//        break;
//    }
//    fflush(stderr);
//    /* If this is a stack overflow then we can't walk the stack, so just show
//      where the error happened */
//    if (EXCEPTION_STACK_OVERFLOW != ExceptionInfo->ExceptionRecord->ExceptionCode)
//    {
//        windows_print_stacktrace(ExceptionInfo->ContextRecord);
//    }
//    else
//    {
//        //addr2line(icky_global_program_name, (void*)ExceptionInfo->ContextRecord->Rip);
//    }
//
//
//    Sleep(3000);
//    return EXCEPTION_EXECUTE_HANDLER;
//}
//
//void set_signal_handler()
//{
//    //std::signal(SIGABRT, SignalHandler);
//    //std::signal(SIGFPE, SignalHandler);
//    //std::signal(SIGILL, SignalHandler);
//    //std::signal(SIGINT, SignalHandler);
//    //std::signal(SIGSEGV, SignalHandler);
//    //std::signal(SIGTERM, SignalHandler);
//    SetUnhandledExceptionFilter(windows_exception_handler);
//}
//
//int overflow() {
//    if (overflow() > 0)
//        return 1;
//}
//
//void Foo(int x) {
//    *(int*)0 = 0;
//    x = 10;
//}
//
//int main()
//{
//
//    __try {
//        set_signal_handler();
//        //try
//         //{
//        int* ptr = new int;
//        delete ptr;
//
//        //std::vector<int> xxx(12);
//        //xxx[1231312];
//
//        Foo(12);
//
//        //*(int*)0 = 0;
//
//        //int of = overflow();
//
//        //volatile int a = 14;
//
//        //volatile int b = a / 0;
//
//        //int c = a / b;
//
//        //printf("b = %d", of);
//
//        //}
//        //catch(...)
//        //{
//        //}
//    }
//    __except (windows_exception_handler(GetExceptionInformation())) {
//        int xxx = 8;
//    }
//
//    std::cout << "happy end \n";
//    Sleep(1000);
//    //system("pause");
//    return 0;
//}