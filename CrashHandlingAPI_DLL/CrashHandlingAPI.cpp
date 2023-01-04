#define BUILD_LIBRARY
#include "CrashHandlingAPI.h"
#include "../ComAPI/ComAPI.h"
#include <Windows.h>
#include <filesystem>
#include <dbghelp.h>
#pragma comment (lib, "dbghelp.lib" )


BOOL CALLBACK MiniDumpCallback(PVOID pParam, const PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput) {
	// Check parameters
	if (pInput == 0)
		return FALSE;

	if (pOutput == 0)
		return FALSE;


	// Process the callbacks 
	BOOL bRet = FALSE;

	switch (pInput->CallbackType) {
	case IncludeModuleCallback:
		// Include the module into the dump 
		bRet = TRUE;
		break;

	case IncludeThreadCallback:
		// Include the thread into the dump 
		bRet = TRUE;
		break;

	case ModuleCallback:
		// Does the module have ModuleReferencedByMemory flag set ? 
		if (!(pOutput->ModuleWriteFlags & ModuleReferencedByMemory)) {
			// No, it does not - exclude it 
			//wprintf(L"Excluding module: %s \n", pInput->Module.FullPath);
			pOutput->ModuleWriteFlags &= (~ModuleWriteModule);
		}
		bRet = TRUE;
		break;

	case ThreadCallback:
		// Include all thread information into the minidump 
		bRet = TRUE;
		break;

	case ThreadExCallback:
		// Include this information 
		bRet = TRUE;
		break;

	case MemoryCallback:
		// We do not include any information here -> return FALSE 
		bRet = FALSE;
		break;

	case CancelCallback:
		break;
	}

	return bRet;
}


void GenerateMiniDump(EXCEPTION_POINTERS* pep, std::wstring) {
	std::wstring path = ComApi::GetPackageFolder();

	if (!std::filesystem::exists(path))
		std::filesystem::create_directories(path);

	HANDLE hFile = CreateFile((path + L"\\MiniDump.dmp").c_str(),
		GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE)) {
		MINIDUMP_EXCEPTION_INFORMATION mdei;

		mdei.ThreadId = GetCurrentThreadId();
		mdei.ExceptionPointers = pep;
		mdei.ClientPointers = FALSE;

		MINIDUMP_CALLBACK_INFORMATION mci;

		mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;
		mci.CallbackParam = 0;

		MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);

		BOOL rv = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
			hFile, mdt, (pep != 0) ? &mdei : 0, 0, &mci);

		if (!rv) {
			//WriteDebug(L"MiniDumpWriteDump failed. Error: %u", GetLastError());
		}
		else {
			//WriteDebug(L"Minidump created");
		}

		CloseHandle(hFile);
	}
	else {
		//WriteDebug(L"CreateFile failed. Error: %u", GetLastError());
	}
}


namespace CrashAPI {
	API void CreateMinidump(EXCEPTION_POINTERS* pep, const wchar_t* path) {
		GenerateMiniDump(pep, path);
	}
}
