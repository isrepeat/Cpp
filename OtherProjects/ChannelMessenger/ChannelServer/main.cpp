#define _DEBUG_READ_FILE_DELAY 500

#include <Helpers/Channel.h>
#include <Helpers/Logger.h>
#include <cassert>
#include <thread>


void TestGreetingWithClient() {
	LOG_FUNCTION_SCOPE("TestGreetingWithClient()");
	enum class Messages {
		None,
		Connect,
		HelloFromClient,
		HelloFromServer,
		UserRequest,
		UserData,
		Stop,
	};
	Channel<Messages> channelServer;
	std::atomic<bool> exitApp = false;

	try {
		channelServer.SetInterruptHandler([&exitApp] {
			LOG_FUNCTION_ENTER("InterruptHandler()");
			});

		channelServer.CreateForAdmin(L"\\\\.\\pipe\\$channelClientServer$",
			[](Channel<Messages>::Msg_t message, Channel<Messages>::WriteFunc Write) {
				switch (message->type) {
				case Messages::Connect: {
					LOG_DEBUG("Connected with ChannelClient \n\n");
					break;
				}
				case Messages::HelloFromClient: {
					LOG_DEBUG("---> got \"HelloFromClient\" \n\n");
					LOG_DEBUG("send \"HelloFromServer\" ... \n");
					Sleep(1000);
					Write({}, Messages::HelloFromServer);
					break;
				}
				case Messages::UserRequest: {
					LOG_DEBUG("---> got \"UserRequest\" \n\n");
					LOG_DEBUG("send \"UserData\" ... \n");
					Sleep(1000);
					Write({}, Messages::UserData);
					break;
				}
				}
				return true;
			});

	}
	catch (PipeError err) {
		LOG_ERROR_S(&channelServer, "Catch PipeError = {}", magic_enum::enum_name(err));
		exitApp = true;
	}

	while (!exitApp) {
		Sleep(10);
	};
}



#include <Helpers/CrashInfo.h>

enum class MiniDumpMessages {
	None,
	Connect,
	PackageFolder,
	AppCenterId,
	AppVersion,
	AppUuid,
	Backtrace,
	ExceptionMessage,
	ExceptionInfo,
	TrashMessage,
	DumpCreated,
};

bool ChannelListenerHandler(Channel<MiniDumpMessages>::Msg_t message, Channel<MiniDumpMessages>::WriteFunc Write) {
	switch (message->type) {
	case MiniDumpMessages::Connect: {
		LOG_DEBUG("[Connect]");
		break;
	}
	case MiniDumpMessages::PackageFolder: {
		auto strData = std::string{ message->payload.begin(), message->payload.end() };
		LOG_DEBUG("[PackageFolder] = {}", strData.c_str());
		break;
	}
	case MiniDumpMessages::AppCenterId: {
		auto strData = std::string{ message->payload.begin(), message->payload.end() };
		LOG_DEBUG("[AppCenterId] = {}", strData.c_str());
		break;
	}
	case MiniDumpMessages::AppVersion: {
		auto strData = std::string{ message->payload.begin(), message->payload.end() };
		LOG_DEBUG("[AppVersion] = {}", strData.c_str());
		break;
	}
	case MiniDumpMessages::AppUuid: {
		auto strData = std::string{ message->payload.begin(), message->payload.end() };
		LOG_DEBUG("[AppUuid] = {}", strData.c_str());
		break;
	}
	case MiniDumpMessages::Backtrace: {
		auto strData = std::string{ message->payload.begin(), message->payload.end() };
		LOG_DEBUG("[Backtrace]:\n{}\n", strData.c_str());
		break;
	}
	case MiniDumpMessages::ExceptionMessage: {
		auto strData = std::string{ message->payload.begin(), message->payload.end() };
		LOG_DEBUG("[ExceptionMessage] = {}", strData.c_str());
		break;
	}
	case MiniDumpMessages::ExceptionInfo: {
		LOG_DEBUG("[ExceptionInfo]");
		//auto crashInfo = DeserializeCrashInfo(message->payload);
		//FixExceptionPointersInCrashInfo(crashInfo);

		//printf("[PIPE] crashInfo: size serialized bytes = %" PRId64 "\n", message->payload.size());
		//printf("[PIPE] crashInfo: number of exceptions = %d \n", crashInfo->numberOfExceptionRecords);
		//printf("[PIPE] crashInfo: hProcess = 0x%08x \n", (DWORD)hProcess);
		//printf("[PIPE] crashInfo: ProcessId = %d \n", processId);
		//printf("[PIPE] creating dump ... \n");
		//GenerateMiniDump(crashInfo, hProcess, processId, crashReportFolder);

		//printf("[PIPE] dump created! \n");
		//Write({}, MiniDumpMessages::DumpCreated); // now crashing process may be closed

		//auto productName = H::StrToWStr(H::RegistryManager::GetRegValue(HKey::LocalMachine, "HARDWARE\\DESCRIPTION\\System\\BIOS", "SystemProductName"));
		//H::FS::WriteFile(crashReportFolder + L"\\" + appUuid.toStdWString() + L"_" + productName, {});

		//QList attachmentDirs{
		//	QDir{QString::fromStdWString(crashReportFolder)},
		//};

		//printf("[PIPE] send report to App Center ... \n");
		//AppCenter::GetInstance().SetApplicationData(appCenterId, appUuid, appVersion);
		//AppCenter::GetInstance().SendCrashReport(exceptionMessage, {}, backtrace, attachmentDirs);
		break;
	}
	case MiniDumpMessages::TrashMessage: {
		LOG_DEBUG("[TrashMessage]");
		break;
	}
	}
	return true;
}

void TestReceiveCrashReportFromClient() {
	LOG_FUNCTION_SCOPE("TestReceiveCrashReportFromClient()");
	Channel<MiniDumpMessages> channelServer;
	std::atomic<bool> exitApp = false;

	try {
		channelServer.SetInterruptHandler([&exitApp] {
			LOG_FUNCTION_ENTER("InterruptHandler()");
			});

		channelServer.Create(L"\\\\.\\pipe\\$channelClientServer$", std::bind(ChannelListenerHandler, std::placeholders::_1, std::placeholders::_2), 20'000);
	}
	catch (PipeError err) {
		LOG_ERROR_S(&channelServer, "Catch PipeError = {}", magic_enum::enum_name(err));
		exitApp = true;
	}

	//auto processAccessFlags = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_DUP_HANDLE;
	//if (HANDLE hProcess = OpenProcess(processAccessFlags, FALSE, processId)) {
	//	if (isUWP) {
	//		auto shortChannelName = std::filesystem::path(minidumpChannelName).filename();
	//		channelMinidump.CreateForUWP(shortChannelName, std::bind(ChannelListenerHandler, std::placeholders::_1, std::placeholders::_2, hProcess), hProcess, 20'000);
	//	}
	//	else {
	//	}
	//}

	while (!exitApp) {
		Sleep(10);
	};
}

void main() {
	_set_error_mode(_OUT_TO_MSGBOX);
	lg::DefaultLoggers::Init(H::ExePath() / "ChannelMessagerLogs\\ChannelServer.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
	LOG_FUNCTION_SCOPE("main()");

	//TestGreetingWithClient();
	TestReceiveCrashReportFromClient();

	system("pause");
	return;
}