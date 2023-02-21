#include "TestClass.h"
#include <Windows.h>
#include "../../../../Shared/Helpers/Helpers.h"
#include "../../../../Shared/Helpers/Conversion.h"
#include <CrashHandling/CrashHandling.h>


LONG __stdcall CustomUnhandledExceptionFilter(EXCEPTION_POINTERS* pep);

namespace LibraryCLR {
	TestClass::TestClass() {
		CrashHandling::RegisterVectorHandler(&CustomUnhandledExceptionFilter);
		Sleep(100);

		*(int*)0 = 0;
		Sleep(1000);
	}
}


LONG __stdcall CustomUnhandledExceptionFilter(EXCEPTION_POINTERS* pep) {
	static int counter = 0;

	counter++;
	std::string exceptionMsg;
	auto hexStr = H::HexToString(pep->ExceptionRecord->ExceptionCode);

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

	case 0xE0434352: {
		//if (counter < 2) 
		//	return EXCEPTION_CONTINUE_SEARCH;

		exceptionMsg = "CLR exception";
		break;
	}

	default:
		//return EXCEPTION_EXECUTE_HANDLER;
		//return EXCEPTION_CONTINUE_EXECUTION;
		return EXCEPTION_CONTINUE_SEARCH;
	}

	std::wstring protcolWithParams = L"/c start trd-minidumpwriter:\"" + H::CreateStringParams({
				{L"-processId", std::to_wstring(GetCurrentProcessId())},
				{L"-sleepAtStart", L"100"},
		}) + L"\"";

	// TODO: hide MinidumpWriter.exe console
	auto res = H::ExecuteCommandLine(protcolWithParams, false, SW_HIDE);

	CrashHandling::OpenMinidumpChannel(pep, H::ExePathW());

	Sleep(10000);

	exit(0);
	return EXCEPTION_EXECUTE_HANDLER;
}