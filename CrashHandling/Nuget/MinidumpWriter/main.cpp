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

template<typename Ret>
Ret Convert(const std::wstring& str) {
	if constexpr (std::is_same_v<Ret, std::wstring>) {
		return str;
	}
	else if constexpr (std::is_same_v<Ret, std::string>) {
		return H::WStrToStr(str);
	}
	else if constexpr (std::is_same_v<Ret, int>) {
		return std::stoi(str);
	}
	else if constexpr (std::is_same_v<Ret, bool>) {
		return str == L"true" ? true : false;
	}
	else {
		static_assert(false, "Type conversion not supported");
	}
}

template<typename T>
void SetValue(const std::map<std::wstring, std::wstring>& mapParams, const std::wstring& searchKey, T& value, bool isRequired = false) {
	if (mapParams.count(searchKey)) {
		value = Convert<T>(mapParams.at(searchKey));
	}
	else {
		if (isRequired) {
			wprintf(L"error: requeied parameter not found = '%s' \n", searchKey);
			throw;
		}
		// not modified, use default value
	}
}



#define PAR_IS_UWP					L"-isUWP"
#define PAR_PROCESS_ID				L"-processId"
#define PAR_SLEEP_AT_START			L"-sleepAtStart"
#define PAR_PROCESS_ACCESS_FLAGS	L"-processAccessFlags"
#define PAR_MINIDUMP_CHANNEL_NAME	L"-minidumpChannelName"

// default values:
auto isUWP = false;
auto processId = 0;
auto sleepAtStart = 0;
auto minidumpChannelName = std::wstring{ L"\\\\.\\pipe\\Local\\channelDumpWriter" };
//auto processAccessFlags = PROCESS_QUERY_LIMITED_INFORMATION;
auto processAccessFlags = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_DUP_HANDLE;

#define CONCAT(a, b) a##b

int _tmain(int argc, _TCHAR* argv[])
{
	for (int i = 0; i < argc; i++) {
		wprintf(L"arg [%d] = %s \n", i, argv[i]);
	}
	wprintf(L"\n\n");

	auto shortChannelName = H::FS::GetFilenameFromPathW(L"\\\\.\\pipe\\Local\\channelDumpWriter");

	auto parseSuccessful = false;

	//if (argc > 3) { // Parse for UWP LaunchFullTrustProcessForCurrentAppAsync(...) where the parsed string is the fourh argument
	//	auto params = H::ParseArgsFromStringToMap(std::wstring{ argv[3] });

	//	wprintf(L"list incomming params [UWP]: \n");
	//	for (auto& param : params) {
	//		wprintf(L"param .. [%s, %s] \n", param.first.c_str(), param.second.c_str());
	//	}

	//	if (params.count(PAR_IS_UWP)) {
	//		isUWP = true;
	//	}

	//}
	//else 
	if (argc > 1) { // Parse for raw call where the parsed string is the second argument
		try {
			auto params = H::ParseArgsFromStringToMap(std::wstring{ argv[1] });

			wprintf(L"list incomming params: \n");
			for (auto& param : params) {
				wprintf(L"param .. [%s, %s] \n", param.first.c_str(), param.second.c_str());
			}

			isUWP = params.count(PAR_IS_UWP);
			SetValue(params, PAR_PROCESS_ID, processId, true);
			SetValue(params, PAR_PROCESS_ACCESS_FLAGS, processAccessFlags);
			SetValue(params, PAR_SLEEP_AT_START, sleepAtStart);
			SetValue(params, PAR_MINIDUMP_CHANNEL_NAME, minidumpChannelName);
			
			parseSuccessful = true;
		}
		catch (std::out_of_range ex) {
			wprintf(L"map 'out of range' exception ... \n");
			parseSuccessful = false;
		}
		catch (...) {
			parseSuccessful = false;
		}
	}

	wprintf(L"\n");
	wprintf(L"list parsed params: \n");
	wprintf(L"-isUWP = %s \n", isUWP ? L"true" : L"false");
	wprintf(L"-processId = %d \n", processId);
	wprintf(L"-sleepAtStart = %d \n", sleepAtStart);
	wprintf(L"-processAccessFlags = %d \n", processAccessFlags);
	wprintf(L"-minidumpChannelName = %s \n", minidumpChannelName.c_str());

	if (!parseSuccessful) {
		wprintf(L"\n");
		wprintf(L"wrong parsed ... \n");
		Sleep(8000);
		return 1;
	}


	channelMinidump.SetLoggerHandler([](std::wstring logMsg) {
		wprintf(L"[channel logs ... %s] \n", logMsg.c_str());
		});

	Sleep(sleepAtStart);

	try {
		if (HANDLE hProcess = OpenProcess(processAccessFlags, FALSE, processId)) {
			if (isUWP) {
				auto shortChannelName = H::FS::GetFilenameFromPathW(minidumpChannelName);
				channelMinidump.CreateForUWP(shortChannelName, [hProcess](Channel<MiniDumpMessages>::ReadFunc Read, Channel<MiniDumpMessages>::WriteFunc Write) {
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
						printf("[PIPE] crashInfo: hProcess = 0x%08x \n", (DWORD)hProcess);
						printf("[PIPE] crashInfo: ProcessId = %d \n", processId);

						printf("[PIPE] dump creating ... \n");
						GenerateMiniDump(crashInfo, hProcess, processId, packageFolder);
						printf("[PIPE] dump created! \n");
						break;
					}
					}
					return true;
					}, hProcess, 20'000);
			}
			else {
				channelMinidump.Create(minidumpChannelName, [hProcess](Channel<MiniDumpMessages>::ReadFunc Read, Channel<MiniDumpMessages>::WriteFunc Write) {
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
						printf("[PIPE] crashInfo: hProcess = 0x%08x \n", (DWORD)hProcess);
						printf("[PIPE] crashInfo: ProcessId = %d \n", processId);

						printf("[PIPE] dump creating ... \n");
						GenerateMiniDump(crashInfo, hProcess, processId, packageFolder);
						printf("[PIPE] dump created! \n");
						break;
					}
					}
					return true;
					}, 20'000);
			}
		}
	}
	catch (PipeError err) {
		wprintf(L"\n");
		printf("channelApplication error = %d \n", static_cast<int>(err));
	}
	catch (...) {
		wprintf(L"\n");
		wprintf(L"last error = %s \n", H::GetLastErrorAsString().c_str());
	}


	printf("\n");
	printf("infinity loop ... \n");
	while (true) {
		Sleep(1000);
		//Beep(500, 500);
	}
	return 0;
}