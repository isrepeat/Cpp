#include "Class1.h"
#include <Windows.h>
#include <string>
#include "../CrashHandlingAPI_DLL/CrashHandlingAPI.h"
#pragma comment (lib, "../x64/Debug/CrashHandlingAPI_DLL.lib")

using namespace WinRT_DLL;
using namespace Platform;


// To debug custom filter wrap your test crash code to __try ... __except(CustomUnhandledExceptionFilter(GetExceptionInformation()))
LONG __stdcall CustomUnhandledExceptionFilter(EXCEPTION_POINTERS* pep) {
	std::string exceptionMsg;
	switch (pep->ExceptionRecord->ExceptionCode) {
	case EXCEPTION_ACCESS_VIOLATION:
		exceptionMsg = "EXCEPTION_ACCESS_VIOLATION";
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		exceptionMsg = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		exceptionMsg = "EXCEPTION_DATATYPE_MISALIGNMENT";
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		exceptionMsg = "EXCEPTION_FLT_DENORMAL_OPERAND";
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		exceptionMsg = "EXCEPTION_FLT_DIVIDE_BY_ZERO";
		break;
	case EXCEPTION_FLT_INEXACT_RESULT:
		exceptionMsg = "EXCEPTION_FLT_INEXACT_RESULT";
		break;
	case EXCEPTION_FLT_INVALID_OPERATION:
		exceptionMsg = "EXCEPTION_FLT_INVALID_OPERATION";
		break;
	case EXCEPTION_FLT_OVERFLOW:
		exceptionMsg = "EXCEPTION_FLT_OVERFLOW";
		break;
	case EXCEPTION_FLT_STACK_CHECK:
		exceptionMsg = "EXCEPTION_FLT_STACK_CHECK";
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		exceptionMsg = "EXCEPTION_ILLEGAL_INSTRUCTION";
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		exceptionMsg = "EXCEPTION_IN_PAGE_ERROR";
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		exceptionMsg = "EXCEPTION_INT_DIVIDE_BY_ZERO";
		break;
	case EXCEPTION_INT_OVERFLOW:
		exceptionMsg = "EXCEPTION_INT_OVERFLOW";
		break;
	default:
		exceptionMsg = "Unrecognized Exception";
	}


	CrashAPI::CreateMinidump(pep, L"D:\\WORK\\TEST\\CrashHandling_MiniDumps");

	exit(0);

	return EXCEPTION_EXECUTE_HANDLER;
}

void Foo() {
	int aaa = 9;
	__try {
		*(int*)0 = 0;
	}
	__except (CustomUnhandledExceptionFilter(GetExceptionInformation())) {
	}

	int bbb = 9;
}

Class1::Class1()
{
	Foo();
}
