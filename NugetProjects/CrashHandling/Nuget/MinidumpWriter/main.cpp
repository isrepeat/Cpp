#include <QtCore/QCoreApplication>
#include <Windows.h>
#include <inttypes.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <memory>
#include "AppCenter.h"
#include "../../../../Shared/Helpers/Helpers.h"
#include "../../../../Shared/Helpers/Channel.h"
#include "../../../../Shared/Helpers/CrashInfo.h"
#include "../CrashHandling/MiniDumpMessages.h"
#include <dbghelp.h>
#pragma comment (lib, "dbghelp.lib" )

#define VERSION L"1.1.2"

// NOTE: use uint8_t (instead whar_t) to be able send serialized structs
Channel<MiniDumpMessages> channelMinidump;
std::wstring crashReportFolder;
std::wstring packageFolder;
QString exceptionMessage;
QString appCenterId;
QString appVersion;
QString backtrace;
QString appUuid;

void GenerateMiniDump(std::shared_ptr<CrashInfo> crashInfo, HANDLE hProcess, int processId, std::wstring path);
bool ChannelListenerHandler(Channel<MiniDumpMessages>::ReadFunc Read, Channel<MiniDumpMessages>::WriteFunc Write, HANDLE hProcess);

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
			wprintf(L"error: requeied parameter not found = '%s' \n", searchKey.c_str());
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
auto processAccessFlags = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_DUP_HANDLE;


int _tmain(int argc, _TCHAR* argv[])
{
	int _argc = 1;
	char _arg0[1] = "";
	char* _argv[2] = { _arg0, nullptr };
	QCoreApplication app(_argc, _argv);

	wprintf(L"Version = %s \n\n", VERSION);

	for (int i = 0; i < argc; i++) {
		wprintf(L"arg [%d] = %s \n", i, argv[i]);
	}
	wprintf(L"\n\n");


	auto parseSuccessful = false;
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
				channelMinidump.CreateForUWP(shortChannelName, std::bind(ChannelListenerHandler, std::placeholders::_1, std::placeholders::_2, hProcess), hProcess, 20'000);
			}
			else {
				channelMinidump.Create(minidumpChannelName, std::bind(ChannelListenerHandler, std::placeholders::_1, std::placeholders::_2, hProcess), 20'000);
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

	return app.exec();
}


void GenerateMiniDump(std::shared_ptr<CrashInfo> crashInfo, HANDLE hProcess, int processId, std::wstring path) {
	if (!std::filesystem::exists(path))
		std::filesystem::create_directories(path);

	HANDLE hFile = CreateFile((path + L"\\MiniDump.dmp").c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE)) {
		MINIDUMP_EXCEPTION_INFORMATION mdei;
		mdei.ThreadId = crashInfo->threadId;
		mdei.ExceptionPointers = &crashInfo->exceptionPointers;
		mdei.ClientPointers = FALSE;

		if (mdei.ExceptionPointers == NULL) {
			wprintf(L"ExceptionPointers == 0 \n");
		}

		MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);

		BOOL rv = MiniDumpWriteDump(hProcess, processId, hFile, mdt, (mdei.ExceptionPointers != NULL) ? &mdei : NULL, NULL, NULL);

		if (!rv) {
			wprintf(L"MiniDumpWriteDump failed. [%u] %s \n", GetLastError(), H::GetLastErrorAsString().c_str());
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

bool ChannelListenerHandler(Channel<MiniDumpMessages>::ReadFunc Read, Channel<MiniDumpMessages>::WriteFunc Write, HANDLE hProcess) {
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
		crashReportFolder = packageFolder + L"\\CrashReport";
		break;
	}
	case MiniDumpMessages::AppCenterId: {
		auto strData = std::string{ reply.readData.begin(), reply.readData.end() };
		printf("[PIPE] appCenterId = %s \n", strData.c_str());
		appCenterId = QUuid::fromString(H::StrToWStr(strData)).toString(QUuid::StringFormat::WithoutBraces);
		break;
	}

	case MiniDumpMessages::AppVersion: {
		auto strData = std::string{ reply.readData.begin(), reply.readData.end() };
		printf("[PIPE] appVersion = %s \n", strData.c_str());
		appVersion = QString::fromStdWString(H::StrToWStr(strData));
		break;
	}

	case MiniDumpMessages::AppUuid: {
		auto strData = std::string{ reply.readData.begin(), reply.readData.end() };
		printf("[PIPE] appUuid = %s \n", strData.c_str());
		appUuid = QUuid::fromString(H::StrToWStr(strData)).toString(QUuid::StringFormat::WithoutBraces);
		break;
	}

	case MiniDumpMessages::Backtrace: {
		auto strData = std::string{ reply.readData.begin(), reply.readData.end() };
		printf("[PIPE] backtrace =\n %s \n", strData.c_str());
		backtrace = QString::fromStdWString(H::StrToWStr(strData));
		break;
	}

	case MiniDumpMessages::ExceptionMessage: {
		auto strData = std::string{ reply.readData.begin(), reply.readData.end() };
		printf("[PIPE] exception message = %s \n", strData.c_str());
		exceptionMessage = QString::fromStdWString(H::StrToWStr(strData));
		break;
	}

	case MiniDumpMessages::ExceptionInfo: {
		auto crashInfo = DeserializeCrashInfo(reply.readData);
		FixExceptionPointersInCrashInfo(crashInfo);

		printf("[PIPE] crashInfo: size serialized bytes = %" PRId64 "\n", reply.readData.size());
		printf("[PIPE] crashInfo: number of exceptions = %d \n", crashInfo->numberOfExceptionRecords);
		printf("[PIPE] crashInfo: hProcess = 0x%08x \n", (DWORD)hProcess);
		printf("[PIPE] crashInfo: ProcessId = %d \n", processId);
		printf("[PIPE] creating dump ... \n");
		GenerateMiniDump(crashInfo, hProcess, processId, crashReportFolder);
		
		printf("[PIPE] dump created! \n");
		Write({}, MiniDumpMessages::DumpCreated); // now crahing process may be closed

		QList attachmentDirs{
			QDir{QString::fromStdWString(crashReportFolder)},
		};

		printf("[PIPE] send report to App Center ... \n");
		AppCenter::GetInstance().SetApplicationData(appCenterId, appUuid, appVersion);
		AppCenter::GetInstance().SendCrashReport(exceptionMessage, {}, backtrace, attachmentDirs);
		
		printf("[PIPE] report sent! \n");
		break;
	}
	}
	return true;
}