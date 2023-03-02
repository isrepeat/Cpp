#include <CrashHandling/CrashHandling.h>
#include "../../../../Shared/ComAPI/ComAPI.h";
#include "../../../../Shared/Helpers/Helpers.h";
#include <Windows.h>
#include <thread>


LONG __stdcall CustomUnhandledExceptionFilter(EXCEPTION_POINTERS* pep) {
	std::wstring exceptionMsg;
	switch (pep->ExceptionRecord->ExceptionCode) {
	case EXCEPTION_ACCESS_VIOLATION:
		exceptionMsg = L"EXCEPTION_ACCESS_VIOLATION";
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		exceptionMsg = L"EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		exceptionMsg = L"EXCEPTION_DATATYPE_MISALIGNMENT";
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		exceptionMsg = L"EXCEPTION_FLT_DENORMAL_OPERAND";
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		exceptionMsg = L"EXCEPTION_FLT_DIVIDE_BY_ZERO";
		break;
	case EXCEPTION_FLT_INEXACT_RESULT:
		exceptionMsg = L"EXCEPTION_FLT_INEXACT_RESULT";
		break;
	case EXCEPTION_FLT_INVALID_OPERATION:
		exceptionMsg = L"EXCEPTION_FLT_INVALID_OPERATION";
		break;
	case EXCEPTION_FLT_OVERFLOW:
		exceptionMsg = L"EXCEPTION_FLT_OVERFLOW";
		break;
	case EXCEPTION_FLT_STACK_CHECK:
		exceptionMsg = L"EXCEPTION_FLT_STACK_CHECK";
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		exceptionMsg = L"EXCEPTION_ILLEGAL_INSTRUCTION";
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		exceptionMsg = L"EXCEPTION_IN_PAGE_ERROR";
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		exceptionMsg = L"EXCEPTION_INT_DIVIDE_BY_ZERO";
		break;
	case EXCEPTION_INT_OVERFLOW:
		exceptionMsg = L"EXCEPTION_INT_OVERFLOW";
		break;

	default:
		return EXCEPTION_CONTINUE_SEARCH;
	}

	auto backtrace = CrashHandling::GetBacktrace(0);
	auto backtraceStr = CrashHandling::BacktraceToString(backtrace);

	std::wstring protcolWithParams = L"/c start test-minidumpwriter:\"" + H::CreateStringParams({
				{L"-processId", std::to_wstring(GetCurrentProcessId())},
				{L"-sleepAtStart", L"100"},
		}) + L"\"";

	auto res = H::ExecuteCommandLine(protcolWithParams, false, SW_HIDE);

	CrashHandling::AppInfo appInfo = {
		ComApi::GetPackageFolder(),
		//L"6d890ba7-55d1-49e7-9f78-1625e0f3b4f9", // TRD
		L"444512e9-b730-4691-a924-2032c785df35", // Restore Files
		L"1.0.0.99",
		L"00000000-0000-0000-0000-000000000001",
		backtraceStr,
		exceptionMsg
	};

	CrashHandling::GenerateCrashReport(pep, appInfo);

	Sleep(5000);

	exit(1);
	return EXCEPTION_EXECUTE_HANDLER;
}


void Foo() {
	//int aaa = 9;

	int* ptr = new int{ 1 };
	delete ptr;

	*ptr = 17;


	//__try {
	//	*(int*)0 = 0;
	//}
	//__except (MyCustomExceptionHandler(GetExceptionInformation())) {
	//}

	int bbb = 9;
}

int main() {
	CrashHandling::RegisterVectorHandler(&CustomUnhandledExceptionFilter);

	Foo();

	while (true) {
		Sleep(100);
	}
	return 0;
}