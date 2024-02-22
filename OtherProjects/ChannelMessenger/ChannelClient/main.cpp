#include <Helpers/PackageProvider.h>
#include <Helpers/Channel.h>
#include <Helpers/Logger.h>
#include <ComAPI/ComAPI.h>
#include <cassert>
#include <thread>


void TestGreetingWithServer() {
	LOG_FUNCTION_SCOPE("TestGreetingWithServer()");
	enum class Messages {
		None,
		Connect,
		HelloFromClient,
		HelloFromServer,
		UserRequest,
		UserData,
		Stop,
	};
	Channel<Messages> channelClient;
	std::atomic<bool> exitApp = false;

	try {
		channelClient.SetInterruptHandler([&exitApp] {
			LOG_FUNCTION_ENTER("InterruptHandler()");
			exitApp = true;
			});

		channelClient.Open(L"\\\\.\\pipe\\$channelClientServer$",
			[](Channel<Messages>::Msg_t message, Channel<Messages>::WriteFunc Write) {
				switch (message->type) {
				case Messages::Connect: {
					LOG_DEBUG("Connected with ChannelServer \n\n");
					LOG_DEBUG("send \"HelloFromClient\" ... \n");
					Sleep(1000);
					Write({}, Messages::HelloFromClient);
					break;
				}
				case Messages::HelloFromServer: {
					LOG_DEBUG("---> got \"HelloFromServer\" \n\n");
					LOG_DEBUG("send \"UserRequest\" ... \n");
					Sleep(1000);
					Write({}, Messages::UserRequest);
					break;
				}
				case Messages::UserData: {
					LOG_DEBUG("---> got \"UserData\" \n\n");
					LOG_DEBUG("break connection ... \n");
					Sleep(1000);
					return false;
				}
				}
				return true;
			});

	}
	catch (PipeError err) {
		LOG_ERROR_S(&channelClient, "Catch PipeError = {}", magic_enum::enum_name(err));
		exitApp = true;
	}

	//channelClient.WaitFinishSendingMessage(Messages::HelloFromClient);

	while (!exitApp) {
		Sleep(10);
	};
}





#include <Helpers/CrashInfo.h>

enum class ExceptionType {
	StructuredException,
	UnhandledException,
};

struct AdditionalInfo {
	std::wstring appCenterId;
	std::wstring appVersion; // if app version passed empty it is used current package app version or 1.0.0.0 (if it is desktop)
	std::wstring appUuid = L"00000000-0000-0000-0000-000000000001"; // unique client app id
	std::wstring backtrace;
	std::wstring exceptionMsg;
	std::wstring channelName = L"\\\\.\\pipe\\Local\\channelDumpWriter";
};

std::function<void(EXCEPTION_POINTERS*, ExceptionType)> gCrashCallback = nullptr;
const uint32_t ClrException = 0xE0434352;
const uint32_t CppException = 0xE06D7363;
bool handledCrashException = false;
bool wasCrashException = false;

LONG WINAPI DefaultVectoredExceptionHandlerFirst(EXCEPTION_POINTERS* pep);
LONG WINAPI DefaultVectoredExceptionHandlerLast(EXCEPTION_POINTERS* pep);

void RegisterDefaultCrashHandler(std::function<void(EXCEPTION_POINTERS*, ExceptionType)> crashCallback) {
	gCrashCallback = crashCallback;
	AddVectoredExceptionHandler(1, DefaultVectoredExceptionHandlerFirst); // additional guard for cathing c++ exception within clr environment
	AddVectoredExceptionHandler(0, DefaultVectoredExceptionHandlerLast);
}

LONG WINAPI DefaultVectoredExceptionHandlerFirst(EXCEPTION_POINTERS* pExceptionPtrs) {
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
	case EXCEPTION_INT_OVERFLOW:
		wasCrashException = true;
		break;

	case ClrException: {
		int xxx = 9;
		break;
	}
	case CppException: {
		int xxx = 9;
		break;
	}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}


LONG WINAPI DefaultVectoredExceptionHandlerLast(EXCEPTION_POINTERS* pExceptionPtrs) {
	if (handledCrashException) {
		printf("DefaultVectoredExceptionHandlerLast() crash exception already handled, ignore ... \n");
		return EXCEPTION_CONTINUE_SEARCH; // ignore next exception
	}

	std::wstring exceptionMsg;
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
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_INT_OVERFLOW:
		break;

	case ClrException: {
		if (wasCrashException) {
			break;
		}
		else {
			return EXCEPTION_CONTINUE_SEARCH;
		}
	}

	case CppException: {
		if (wasCrashException) {
			break;
		}
		else {
			return EXCEPTION_CONTINUE_SEARCH;
		}
	}

	default:
		return EXCEPTION_CONTINUE_SEARCH;
	}

	printf("DefaultVectoredExceptionHandlerLast() call crash callback ...\n");
	handledCrashException = true;
	if (gCrashCallback) {
		gCrashCallback(pExceptionPtrs, ExceptionType::StructuredException);
	}

	Sleep(10'000);
	exit(0);
}

void DoAccessViolation() {
	__try {
		*(int*)0 = 0;
	}
	__finally {
		assert(false);
	}
}

void TestSendCrashReportToServer() {
	LOG_FUNCTION_SCOPE("TestSendCrashReportToServer()");

	RegisterDefaultCrashHandler([](EXCEPTION_POINTERS* pExceptionPtrs, ExceptionType exType) {
		LOG_FUNCTION_SCOPE("CrashCallback()");
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
		Channel<MiniDumpMessages> channelClient;
		std::atomic<bool> exitApp = false;

		try {
			channelClient.SetInterruptHandler([&exitApp] {
				LOG_FUNCTION_ENTER("InterruptHandler()");
				exitApp = true;
				});

			AdditionalInfo additionalInfo;
			additionalInfo.appCenterId = L"6d890ba7-55d1-49e7-9f78-1625e0f3b4f9";
			additionalInfo.appVersion = L""; // detect automatically
			additionalInfo.appUuid = L"11000000-0000-0000-0000-000000000001";

			auto packageFolder = H::PackageProvider::IsRunningUnderPackage() ? ComApi::GetPackageFolder() : H::ExePath();

			channelClient.Open(L"\\\\.\\pipe\\$channelClientServer$",
				[&](Channel<MiniDumpMessages>::Msg_t message, Channel<MiniDumpMessages>::WriteFunc Write) {
					switch (message->type) {
					case MiniDumpMessages::Connect: {
						std::string strData = H::WStrToStr(packageFolder);
						Write({ strData.begin(), strData.end() }, MiniDumpMessages::PackageFolder);

						strData = H::WStrToStr(additionalInfo.appCenterId);
						Write({ strData.begin(), strData.end() }, MiniDumpMessages::AppCenterId);

						strData = H::WStrToStr(additionalInfo.appVersion);
						Write({ strData.begin(), strData.end() }, MiniDumpMessages::AppVersion);

						strData = H::WStrToStr(additionalInfo.appUuid);
						Write({ strData.begin(), strData.end() }, MiniDumpMessages::AppUuid);

						strData = H::WStrToStr(additionalInfo.backtrace);
						Write({ strData.begin(), strData.end() }, MiniDumpMessages::Backtrace);

						strData = H::WStrToStr(additionalInfo.exceptionMsg);
						Write({ strData.begin(), strData.end() }, MiniDumpMessages::ExceptionMessage);

						auto crashInfo = std::make_shared<CrashInfo>();
						crashInfo->threadId = GetCurrentThreadId();
						FillCrashInfoWithExceptionPointers(crashInfo, pExceptionPtrs);
						auto serializedData = SerializeCrashInfo(crashInfo);
						Write(std::move(serializedData), MiniDumpMessages::ExceptionInfo);
						
						Write({ 1, 2, 3, 4, 5, 6 }, MiniDumpMessages::TrashMessage);
						break;
					}
					}
					return true;
				}, 10'000);
			//channelClient.WaitFinishSendingMessage(Messages::HelloFromClient);
		}
		catch (PipeError err) {
			LOG_ERROR_S(&channelClient, "Catch PipeError = {}", magic_enum::enum_name(err));
			exitApp = true;
		}
	

		while (!exitApp) {
			Sleep(10);
		};
		});

	DoAccessViolation();
}


void main() {
	_set_error_mode(_OUT_TO_MSGBOX);
	lg::DefaultLoggers::Init(H::ExePath() / "ChannelMessagerLogs\\ChannelClient.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
	LOG_FUNCTION_SCOPE("main()");
	
	//TestGreetingWithServer();
	TestSendCrashReportToServer();

	system("pause");
	return;
}