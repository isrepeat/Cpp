#define BUILD_LIBRARY
#include "CrashHandlingAPI.h"
#include "../../Shared/Helpers/Helpers.h"
#include "../../Shared/Helpers/Channel.h"
#include "../../Shared/ComAPI/ComAPI.h"
#include <Windows.h>
#include <filesystem>
#include <dbghelp.h>
#pragma comment (lib, "dbghelp.lib" )

enum class MiniDumpMessages {
	None,
	Connect,
	ThreadId,
	ExceptionInfo,
};

Channel<MiniDumpMessages> channelMinidump;

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

//LONG __stdcall CustomExceptionHandler(EXCEPTION_POINTERS* pep)
//{
//	printf("MyCustomFilter start \n");
//	GenerateMiniDump(pep, L"");
//	printf("MyCustomFilter end \n");
//
//	exit(1);
//	return EXCEPTION_EXECUTE_HANDLER; //EXCEPTION_CONTINUE_EXECUTION
//}


namespace CrashAPI {
	API void CreateMinidump(EXCEPTION_POINTERS* pep, const wchar_t* path) {
		GenerateMiniDump(pep, path);
	}

	API void RegisterVectorHandler(PVECTORED_EXCEPTION_HANDLER handler) {
		int xxx = 9;
		//AddVectoredExceptionHandler(0, CustomExceptionHandler);
		AddVectoredExceptionHandler(0, handler);
	}

	API bool ExecuteCommandLine(const wchar_t* parameters, bool admin, DWORD showFlag) {
        return H::ExecuteCommandLine(parameters, admin, showFlag);
    }

	API void CreateMinidumpChannel(int threadId, EXCEPTION_POINTERS* pep) {
		channelMinidump.Create(L"\\\\.\\pipe\\Local\\MyPipe", [threadId, pep](Channel<MiniDumpMessages>::ReadFunc Read, Channel<MiniDumpMessages>::WriteFunc Write) {
			auto reply = Read();
			switch (reply.type)
			{
			case MiniDumpMessages::Connect:
				Write({ (uint8_t)threadId }, MiniDumpMessages::ThreadId);
				break;
			}
			
			return true;
			});

		channelMinidump.WaitFinishSendingMessage(MiniDumpMessages::ThreadId);
	}

	void OpenMinidumpChannel(int threadId, EXCEPTION_POINTERS* pep) {
		channelMinidump.Open(L"\\\\.\\pipe\\Local\\MyPipe", [threadId, pep](Channel<MiniDumpMessages>::ReadFunc Read, Channel<MiniDumpMessages>::WriteFunc Write) {
			auto reply = Read();
			switch (reply.type)
			{
			case MiniDumpMessages::Connect:
				Write({ (uint8_t)threadId }, MiniDumpMessages::ThreadId);
				break;
			}

			return true;
			});

		channelMinidump.WaitFinishSendingMessage(MiniDumpMessages::ThreadId);
	}
}
