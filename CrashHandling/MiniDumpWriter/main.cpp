#include <iostream>
#include <Windows.h>
#include <string>
#include "../../Shared/Helpers/Helpers.h"
#include "../../Shared/Helpers/Channel.h"


std::wstring GetLastErrorAsString()
{
	//Get the error message ID, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		return std::wstring(); //No error message has been recorded
	}

	LPWSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	std::wstring message(messageBuffer, size);

	//Free the Win32's string's buffer.
	LocalFree(messageBuffer);

	return message;
}

enum class MiniDumpMessages {
    None,
    Connect,
    ThreadId,
    ExceptionInfo,
};

Channel<MiniDumpMessages> channelMinidump;

int _tmain(int argc, _TCHAR* argv[])
{
    for (int i = 0; i < argc; i++) {
        wprintf(L"arg [%d] = %s \n", i, argv[i]);
    }
	wprintf(L"\n\n");

	int processId = 0;
	if (argc > 1) {
		auto listArgs = H::ParseArgsFromString(std::wstring{ argv[1] });

		for (auto& arg : listArgs) {
	        wprintf(L"parsed .. %s \n", arg.c_str());
		}

		if (listArgs.size() > 1 && listArgs[0] == L"-threadId") {
			processId = std::stoi(listArgs[1]);
		}
	}


	if (!processId)
		return 1;


	//int processId = 25340;
	////HANDLE hProcess = GetCurrentProcess();
	//HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, processId);

	//std::wstring pipeNameUWP;

	//HANDLE hToken = nullptr;
	//if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {

	//	GetTokenUserSid(hToken);

	//	if (auto pTokenAppContainerInfo = GetTokenInfo<TOKEN_APPCONTAINER_INFORMATION>(hToken, ::TokenAppContainerSid)) {
	//		if (auto pTokenSessingId = GetTokenInfo<ULONG>(hToken, ::TokenSessionId)) {
	//			LocalPtr<WCHAR> pStr;
	//			if (ConvertSidToStringSidW(pTokenAppContainerInfo->TokenAppContainer, pStr.ReleaseAndGetAdressOf())) {
	//				int aaa = 11;

	//				pipeNameUWP = L"\\\\?\\pipe\\Sessions\\" + std::to_wstring(*pTokenSessingId) + L"\\AppContainerNamedObjects\\" + pStr.get() + L"\\MyPipe";
	//			}
	//			else {
	//				auto err = GetLastErrorAsString();
	//				int xx = 9;
	//			}
	//		}
	//	}
	//}

	
	//int processId = 25340;

	try {
		if (HANDLE hProcessUWP = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, processId))
		{
			channelMinidump.Create(L"MyPipe", [processId](Channel<MiniDumpMessages>::ReadFunc Read, Channel<MiniDumpMessages>::WriteFunc Write) {
				auto reply = Read();
				switch (reply.type) {
				case MiniDumpMessages::Connect: {
					printf("[PIPE] processId = %d \n", processId);
					break;
				}

				case MiniDumpMessages::ThreadId: {
					int threadId = reply.readData[0];
					printf("[PIPE] threadId = %d \n", threadId);
					break;
				}
				}
				return true;
				}, 0, true, true, hProcessUWP);
		}

	}
	catch (PipeError err) {
		printf("channelApplication error = %d", static_cast<int>(err));
	}


	Beep(1000, 1000);
    Sleep(10000);
    return 0;
}