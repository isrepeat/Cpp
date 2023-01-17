#include "../../../Shared/Helpers/Helpers.h"
#include "../../../Shared/Helpers/Channel.h"
#include "../../../Shared/Helpers/CrashInfo.h"
#include "../CrashHandling/MiniDumpMessages.h"
#include <filesystem>
#include <Windows.h>
#include <iostream>
#include <string>
#include <dbghelp.h>
#pragma comment (lib, "dbghelp.lib" )


Channel<MiniDumpMessages> channelMinidump;
std::wstring packageFolder;


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


//void GenerateMiniDump(EXCEPTION_POINTERS* pep, std::wstring path) {
void GenerateMiniDump(CrashInfo& crashInfo, HANDLE hProcess, int processId, std::wstring path) {
	if (!std::filesystem::exists(path))
		std::filesystem::create_directories(path);

	HANDLE hFile = CreateFile((path + L"\\MiniDump.dmp").c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE)) {
		MINIDUMP_EXCEPTION_INFORMATION mdei;

		mdei.ThreadId = crashInfo.threadId;
		mdei.ExceptionPointers = &crashInfo.exceptionPointers;
		mdei.ClientPointers = FALSE;

		MINIDUMP_CALLBACK_INFORMATION mci;

		mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;
		mci.CallbackParam = 0;

		MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);

		//BOOL rv = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, mdt, (mdei.ExceptionPointers != 0) ? &mdei : 0, 0, &mci);
		BOOL rv = MiniDumpWriteDump(hProcess, processId, hFile, mdt, (mdei.ExceptionPointers != 0) ? &mdei : 0, 0, &mci);

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



//std::wstring GetLastErrorAsString()
//{
//	//Get the error message ID, if any.
//	DWORD errorMessageID = ::GetLastError();
//	if (errorMessageID == 0) {
//		return std::wstring(); //No error message has been recorded
//	}
//
//	LPWSTR messageBuffer = nullptr;
//
//	//Ask Win32 to give us the string version of that message ID.
//	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
//	size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
//		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);
//
//	//Copy the error message into a std::string.
//	std::wstring message(messageBuffer, size);
//
//	//Free the Win32's string's buffer.
//	LocalFree(messageBuffer);
//
//	return message;
//}





int _tmain(int argc, _TCHAR* argv[])
{
	for (int i = 0; i < argc; i++) {
		wprintf(L"arg [%d] = %s \n", i, argv[i]);
	}
	wprintf(L"\n\n");

	int processId = 0;
	std::wstring minidumpPath;
	if (argc > 1) {
		auto listArgs = H::ParseArgsFromString(std::wstring{ argv[1] });

		for (auto& arg : listArgs) {
			wprintf(L"parsed .. %s \n", arg.c_str());
		}

		if (listArgs.size() > 3 && listArgs[0] == L"-processId" && listArgs[2] == L"-minidumpPath") {
			processId = std::stoi(listArgs[1]);
			minidumpPath = listArgs[3];
		}
		else {
			wprintf(L"wrong parsed ... ");
			Sleep(2000);
			return 1;
		}
	}


	//Sleep(15'000);

	try {
		Sleep(100);
		//if (HANDLE hProcessUWP = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, processId))
		if (HANDLE hProcessUWP = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_DUP_HANDLE, FALSE, processId))
		{
			channelMinidump.CreateForUWP(L"channelDumpWriter", [hProcessUWP, processId](Channel<MiniDumpMessages>::ReadFunc Read, Channel<MiniDumpMessages>::WriteFunc Write) {
				auto reply = Read();
				switch (reply.type) {
				case MiniDumpMessages::Connect: {
					printf("[PIPE] processId = %d \n", processId);
					break;
				}

				case MiniDumpMessages::PackageFolder: {
					auto strData = std::string{ reply.readData.begin(), reply.readData.end() };
					printf("[PIPE] packageFolder = %s \n", strData.c_str());
					packageFolder = H::StrToWStr(strData);
					break;
				}

				case MiniDumpMessages::ExceptionInfo: {
					auto crashInfo = DeserializeCrashInfo(reply.readData);
					FixExceptionPointersInCrashInfo(crashInfo);
					printf("[PIPE] crashInfo: number of exceptions = %d \n", crashInfo.numberOfExceptionRecords);
					printf("[PIPE] crashInfo: hProcessUWP = 0x%08x \n", (DWORD)hProcessUWP);
					printf("[PIPE] crashInfo: ProcessId = %d \n", processId);

					GenerateMiniDump(crashInfo, hProcessUWP, processId, packageFolder);
					break;
				}
				}
				return true;
				}, hProcessUWP, 20'000);
		}
	}
	catch (PipeError err) {
		printf("channelApplication error = %d", static_cast<int>(err));
	}


	while (true) {
		Sleep(1000);
		//Beep(500, 500);
	}
	return 0;
}