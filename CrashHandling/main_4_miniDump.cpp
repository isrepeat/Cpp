///////////////////////////////////////////////////////////////////////////////
//
// MiniDump.cpp 
//
// Sample approach to collecting data with MiniDumpWriteDump and MiniDumpCallback 
// 
// Author: Oleg Starodumov (www.debuginfo.com)
//
//


///////////////////////////////////////////////////////////////////////////////
// Include files 
//

#include <windows.h>
#include <tchar.h>
#include <dbghelp.h>
#include <stdio.h>
#include <crtdbg.h>


///////////////////////////////////////////////////////////////////////////////
// Directives 
//

#pragma comment ( lib, "dbghelp.lib" )


///////////////////////////////////////////////////////////////////////////////
// Function declarations 
//

void CreateMiniDump(EXCEPTION_POINTERS* pep);

BOOL CALLBACK MyMiniDumpCallback(
	PVOID                            pParam,
	const PMINIDUMP_CALLBACK_INPUT   pInput,
	PMINIDUMP_CALLBACK_OUTPUT        pOutput
);


///////////////////////////////////////////////////////////////////////////////
// Test data and code 
//

struct A
{
	int a;

	A()
		: a(0) {}

	void Print()
	{
		_tprintf(_T("a: %d\n"), a);
	}
};

struct B
{
	A* pA;

	B()
		: pA(0) {}

	void Print()
	{
		_tprintf(_T("pA: %x\n"), pA);
		pA->Print();
	}

};

void DoWork()
{
	B* pB = new B(); // but forget to initialize B::pA 

	*(int*)0 = 0;
	pB->Print(); // here it should crash 
}


///////////////////////////////////////////////////////////////////////////////
// main() function 
//

//LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS* ExceptionInfo)
//{
//	printf("Catch unhandled exception");
//	Sleep(1000);
//	switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
//	{
//	case EXCEPTION_ACCESS_VIOLATION:
//		fputs("Error: EXCEPTION_ACCESS_VIOLATION\n", stdout);
//		break;
//	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
//		fputs("Error: EXCEPTION_ARRAY_BOUNDS_EXCEEDED\n", stderr);
//		break;
//	case EXCEPTION_BREAKPOINT:
//		fputs("Error: EXCEPTION_BREAKPOINT\n", stderr);
//		break;
//	case EXCEPTION_DATATYPE_MISALIGNMENT:
//		fputs("Error: EXCEPTION_DATATYPE_MISALIGNMENT\n", stderr);
//		break;
//	case EXCEPTION_FLT_DENORMAL_OPERAND:
//		fputs("Error: EXCEPTION_FLT_DENORMAL_OPERAND\n", stderr);
//		break;
//	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
//		fputs("Error: EXCEPTION_FLT_DIVIDE_BY_ZERO\n", stdout);
//		break;
//	case EXCEPTION_FLT_INEXACT_RESULT:
//		fputs("Error: EXCEPTION_FLT_INEXACT_RESULT\n", stderr);
//		break;
//	case EXCEPTION_FLT_INVALID_OPERATION:
//		fputs("Error: EXCEPTION_FLT_INVALID_OPERATION\n", stderr);
//		break;
//	case EXCEPTION_FLT_OVERFLOW:
//		fputs("Error: EXCEPTION_FLT_OVERFLOW\n", stderr);
//		break;
//	case EXCEPTION_FLT_STACK_CHECK:
//		fputs("Error: EXCEPTION_FLT_STACK_CHECK\n", stderr);
//		break;
//	case EXCEPTION_FLT_UNDERFLOW:
//		fputs("Error: EXCEPTION_FLT_UNDERFLOW\n", stderr);
//		break;
//	case EXCEPTION_ILLEGAL_INSTRUCTION:
//		fputs("Error: EXCEPTION_ILLEGAL_INSTRUCTION\n", stderr);
//		break;
//	case EXCEPTION_IN_PAGE_ERROR:
//		fputs("Error: EXCEPTION_IN_PAGE_ERROR\n", stderr);
//		break;
//	case EXCEPTION_INT_DIVIDE_BY_ZERO:
//		fputs("Error: EXCEPTION_INT_DIVIDE_BY_ZERO\n", stderr);
//		break;
//	case EXCEPTION_INT_OVERFLOW:
//		fputs("Error: EXCEPTION_INT_OVERFLOW\n", stderr);
//		break;
//	case EXCEPTION_INVALID_DISPOSITION:
//		fputs("Error: EXCEPTION_INVALID_DISPOSITION\n", stderr);
//		break;
//	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
//		fputs("Error: EXCEPTION_NONCONTINUABLE_EXCEPTION\n", stderr);
//		break;
//	case EXCEPTION_PRIV_INSTRUCTION:
//		fputs("Error: EXCEPTION_PRIV_INSTRUCTION\n", stderr);
//		break;
//	case EXCEPTION_SINGLE_STEP:
//		fputs("Error: EXCEPTION_SINGLE_STEP\n", stderr);
//		break;
//	case EXCEPTION_STACK_OVERFLOW:
//		fputs("Error: EXCEPTION_STACK_OVERFLOW\n", stderr);
//		break;
//	default:
//		fputs("Error: Unrecognized Exception\n", stderr);
//		break;
//	}
//	fflush(stderr);
//	/* If this is a stack overflow then we can't walk the stack, so just show
//	  where the error happened */
//	if (EXCEPTION_STACK_OVERFLOW != ExceptionInfo->ExceptionRecord->ExceptionCode)
//	{
//		windows_print_stacktrace(ExceptionInfo->ContextRecord);
//	}
//	else
//	{
//		//addr2line(icky_global_program_name, (void*)ExceptionInfo->ContextRecord->Rip);
//	}
//
//
//	Sleep(3000);
//	return EXCEPTION_EXECUTE_HANDLER;
//}

LONG __stdcall MyCustomFilter(EXCEPTION_POINTERS* pep)
{
	printf("MyCustomFilter start \n");
	CreateMiniDump(pep);
	printf("MyCustomFilter end \n");

	return EXCEPTION_EXECUTE_HANDLER;
}


int main(int argc, char* argv[])
{
	//SetUnhandledExceptionFilter(MyCustomFilter);
	__try
	{
		DoWork();
	}
	//__except (CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	__except (MyCustomFilter(GetExceptionInformation()))
	{
		int xxx = 9;
	}

	system("pause");
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Minidump creation function 
//

void CreateMiniDump(EXCEPTION_POINTERS* pep)
{
	printf("CreateMiniDump start \n");
	// Open the file 

	HANDLE hFile = CreateFile(_T("MiniDump.dmp"), GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
	{
		// Create the minidump 

		MINIDUMP_EXCEPTION_INFORMATION mdei;

		mdei.ThreadId = GetCurrentThreadId();
		mdei.ExceptionPointers = pep;
		mdei.ClientPointers = FALSE;

		MINIDUMP_CALLBACK_INFORMATION mci;

		mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback;
		mci.CallbackParam = 0;

		MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);

		printf("Write mini dump start \n");
		BOOL rv = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
			hFile, mdt, (pep != 0) ? &mdei : 0, 0, &mci);

		printf("Write mini dump end \n");

		if (!rv)
			_tprintf(_T("MiniDumpWriteDump failed. Error: %u \n"), GetLastError());
		else
			_tprintf(_T("Minidump created.\n"));

		// Close the file 

		CloseHandle(hFile);

	}
	else
	{
		_tprintf(_T("CreateFile failed. Error: %u \n"), GetLastError());
	}

	printf("CreateMiniDump end \n");
}


///////////////////////////////////////////////////////////////////////////////
// Custom minidump callback 
//

BOOL CALLBACK MyMiniDumpCallback(
	PVOID                            pParam,
	const PMINIDUMP_CALLBACK_INPUT   pInput,
	PMINIDUMP_CALLBACK_OUTPUT        pOutput
)
{
	printf("MyMiniDumpCallback start \n");
	BOOL bRet = FALSE;


	// Check parameters 

	if (pInput == 0)
		return FALSE;

	if (pOutput == 0)
		return FALSE;


	// Process the callbacks 

	switch (pInput->CallbackType)
	{
	case IncludeModuleCallback:
	{
		// Include the module into the dump 
		bRet = TRUE;
	}
	break;

	case IncludeThreadCallback:
	{
		// Include the thread into the dump 
		bRet = TRUE;
	}
	break;

	case ModuleCallback:
	{
		// Does the module have ModuleReferencedByMemory flag set ? 

		if (!(pOutput->ModuleWriteFlags & ModuleReferencedByMemory))
		{
			// No, it does not - exclude it 

			wprintf(L"Excluding module: %s \n", pInput->Module.FullPath);

			pOutput->ModuleWriteFlags &= (~ModuleWriteModule);
		}

		bRet = TRUE;
	}
	break;

	case ThreadCallback:
	{
		// Include all thread information into the minidump 
		bRet = TRUE;
	}
	break;

	case ThreadExCallback:
	{
		// Include this information 
		bRet = TRUE;
	}
	break;

	case MemoryCallback:
	{
		// We do not include any information here -> return FALSE 
		bRet = FALSE;
	}
	break;

	case CancelCallback:
		int xxx = 9;
		break;
	}

	printf("MyMiniDumpCallback end \n");
	return bRet;
}
