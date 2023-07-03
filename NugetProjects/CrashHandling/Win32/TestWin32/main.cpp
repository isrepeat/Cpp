//#include <CrashHandling/CrashHandling.h>
#include "../../Nuget/CrashHandling/CrashHandling.h"
#include "../../../../Shared/ComAPI/ComAPI.h";
#include "../../../../Shared/Helpers/Helpers.h";
#include "../../../../Shared/Helpers/RegistryManager.h";
#include "../../../../Shared/Helpers/PackageProvider.h";
#include <Windows.h>
#include <thread>


#pragma comment(lib, "User32.lib")

const uint32_t ClrException = 0xE0434352;
const uint32_t CppException = 0xE06D7363;

LONG WINAPI MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionPtrs)
{
	switch (pExceptionPtrs->ExceptionRecord->ExceptionCode) {
	case EXCEPTION_ACCESS_VIOLATION:
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
	case EXCEPTION_DATATYPE_MISALIGNMENT:
	case EXCEPTION_FLT_DENORMAL_OPERAND:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_INEXACT_RESULT:
	case EXCEPTION_FLT_INVALID_OPERATION:
	case EXCEPTION_FLT_OVERFLOW:
	case EXCEPTION_FLT_STACK_CHECK:
	case EXCEPTION_ILLEGAL_INSTRUCTION:
	case EXCEPTION_IN_PAGE_ERROR:
	case EXCEPTION_INT_OVERFLOW: {
		int xxx = 9;
		//Beep(2000, 1000);
		MessageBoxA(NULL, (LPCSTR)"Structured exception ...", (LPCSTR)"unhandel exception filter", MB_ICONINFORMATION | MB_DEFBUTTON2);
		break;
	}

	case ClrException: {
		int xxx = 9;
		MessageBoxA(NULL, (LPCSTR)"CLR exception ...", (LPCSTR)"unhandel exception filter", MB_ICONINFORMATION | MB_DEFBUTTON2);
		//Beep(1000, 500);
		//Beep(1000, 500);
		break;
	}
	case CppException: {
		MessageBoxA(NULL, (LPCSTR)"Cpp exception ...", (LPCSTR)"unhandel exception filter", MB_ICONINFORMATION | MB_DEFBUTTON2);
		int xxx = 9;
		//Beep(400, 200);
		//Beep(400, 200);
		//Beep(400, 200);
		break;
	}
	}

	// Do something, for example generate error report

	//..

	// Execute default exception handler next
	return EXCEPTION_EXECUTE_HANDLER;
}


void Foo() {
	//int aaa = 9;

	int* ptr = new int{ 1 };
	delete ptr;

	//*ptr = 17;
	
	
	//__try
	//{
		throw 111;
	//}
	//__except (MyUnhandledExceptionFilter(GetExceptionInformation()))
	//{
	//	Sleep(10);
	//}


	//__try {
	//	*(int*)0 = 0;
	//}
	//__except (MyCustomExceptionHandler(GetExceptionInformation())) {
	//}

	int bbb = 9;
}

int main() {
	//SetUnhandledExceptionFilter(&MyUnhandledExceptionFilter);

	CrashHandling::RegisterDefaultCrashHandler([](EXCEPTION_POINTERS* pExceptionPtrs, CrashHandling::ExceptionType exType) {
		std::wstring exceptionMsg;
		switch (exType) {
		case CrashHandling::ExceptionType::StructuredException: {
			switch (pExceptionPtrs->ExceptionRecord->ExceptionCode) {
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
			}
			printf("*** Caught structured exception *** \n");
			break;
		}
		case CrashHandling::ExceptionType::UnhandledException: {
			MessageBoxA(NULL, (LPCSTR)"Caught unhandled exception !!!", (LPCSTR)"Unhandled exception filter", MB_ICONINFORMATION | MB_DEFBUTTON2);
			exceptionMsg = L"UNHANDLED_EXCEPTION";
			printf("*** Caught unhandled exception *** \n");
			break;
		}
		}

		auto backtrace = CrashHandling::GetBacktrace(0);
		auto backtraceStr = CrashHandling::BacktraceToString(backtrace);

		CrashHandling::AdditionalInfo additionalInfo = {
		//ComApi::GetPackageFolder(),
		H::ExePathW(),
		L"6d890ba7-55d1-49e7-9f78-1625e0f3b4f9", // TRD
		//L"444512e9-b730-4691-a924-2032c785df35", // Restore Files
		L"1.0.0.99",
		L"11000000-0000-0000-0000-000000000001",
		backtraceStr,
		exceptionMsg
		};

		additionalInfo.backtrace = backtraceStr;
		additionalInfo.exceptionMsg = exceptionMsg;
		CrashHandling::GenerateCrashReport(pExceptionPtrs, additionalInfo, L"test-minidumpwriter", { {L"-debug", L""} });
		});

	//auto th = std::thread([] {
	//	//throw 123;
	//	Sleep(10'000);
	//	});

	//th.detach();
	////if (th.joinable())
	////	th.join();

	Sleep(100);
	
	Foo();

	Sleep(100);
	//auto productName = H::RegistryManager::GetRegValue(HKey::LocalMachine, "HARDWARE\\DESCRIPTION\\System\\BIOS", "SystemProductName");

	while (true) {
		Sleep(800);
		printf("...\n");
	}
	return 0;
}