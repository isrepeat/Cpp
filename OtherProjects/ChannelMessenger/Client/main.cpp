#include <CrashHandling/CrashHandling.h>
#include <MagicEnum/MagicEnum.h>
#include <Helpers/Channel.h>
#include <Helpers/HLogger.h>
#include <cassert>
#include <thread>

void RegisterCrashHandler();


enum class Messages {
	None,
	Connect,
	FrameRequest,
	FrameData,
	UserInput,
	Stop,
};

Channel<Messages> channelClient;
std::atomic<bool> exitApp = false;
std::mutex mx;


void main() {
	RegisterCrashHandler();

	lg::DefaultLoggers::Init(H::ExePathW() + L"\\Logs\\ClientLogs.log");
	LOG_DEBUG("Start client");

	//*(int*)0 = 0;

	try {
		channelClient.SetInterruptHandler([] {
			exitApp = true;
			});

		channelClient.Open(L"\\\\.\\pipe\\$channelClientServer$",
			[](Channel<Messages>::ReadFunc Read, Channel<Messages>::WriteFunc Write) {
				auto reply = Read();
				//switch (reply.type) {
				//case Messages::Connect: {
				//	LOG_DEBUG("[Connect] Connected with ChannelServer");
				//	break;
				//}
				//}
				return true;
			});

	}
	catch (PipeError err) {
		LOG_ERROR_D("pipe error = {}", magic_enum::enum_name(err));
		exitApp = true;
	}



	auto thFrameRequest = std::thread([] {
		//while (!exitApp) {
		//	//std::unique_lock lk{ mx };
		//	//std::vector<uint8_t> message = { 1,2,3,4,5,6,7,8,9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
		//	channelClient.Write({}, Messages::FrameRequest);


		//	////std::vector<uint8_t> message(300 * 1024); // 10 KB
		//	////for (int i = 0; i < message.size(); i++) {
		//	////	message[i] = i;
		//	////}
		//	//channelClient.Write({ message.begin(), message.end() }, Messages::UserInput);
		//}
		});

	Sleep(100);
	
	std::vector<uint8_t> g_message = { 0,1,2,3,4 };

	auto thUserInput = std::thread([&g_message] {
		while (!exitApp) {
			//std::unique_lock lk{ mx };
			//std::vector<uint8_t> message = { 1 };
			//std::vector<uint8_t> message = { 1,2,3,4,5,6,7,8,9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
			
			std::vector<uint8_t> message = { 0,1,2,3,4 };
			//uint8_t* data = new uint8_t[5]{ 0,1,2,3,4 };

			int aa = 1;
			int bb = 2;
			//channelClient.Write({ 0,1,2,3,4 }, Messages::UserInput);
			//channelClient.Write({ message.begin(), message.end() }, Messages::UserInput);
			//channelClient.Write(std::move(message), Messages::UserInput);

			channelClient.Write(message, Messages::UserInput);
			//channelClient.Write(g_message, Messages::UserInput);
			//channelClient.Write2(data, 5, Messages::UserInput);			
			int cc = aa + bb;
			int dd = 1 - cc;
			//channelClient.Write({}, Messages::UserInput);
			//Sleep(10);
		}
		});


	while (!exitApp) {
		Sleep(10);
	};

	if (thFrameRequest.joinable()) {
		thFrameRequest.join();
	}

	if (thUserInput.joinable()) {
		thUserInput.join();
	}


	LOG_DEBUG("Exit server");
	//Sleep(3000);
	return;
}

void RegisterCrashHandler() {
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
			break;
		}
		case CrashHandling::ExceptionType::UnhandledException: {
			exceptionMsg = L"UNHANDLED_EXCEPTION";
			break;
		}
		}

		LOG_DEBUG(L"*** Caugth {} ***", exceptionMsg);


		auto backtrace = CrashHandling::GetBacktrace(0);
		auto backtraceStr = CrashHandling::BacktraceToString(backtrace);

		CrashHandling::AdditionalInfo additionalInfo = {
		L"6d890ba7-55d1-49e7-9f78-1625e0f3b4f9", // appCenterId (TeamRemoteDesktop)
		//L"444512e9-b730-4691-a924-2032c785df35", // appCenterId (Restore Files)
		L"1.0.0.99", // appVersion
		//L"", // appVersion
		L"11000000-0000-0000-0000-000000000001", // appUuid
		backtraceStr, // backtrace
		exceptionMsg // exceptionMsg
		};

		additionalInfo.backtrace = backtraceStr;
		additionalInfo.exceptionMsg = exceptionMsg;
		CrashHandling::GenerateCrashReport(pExceptionPtrs, additionalInfo, L"trd-minidumpwriter", { {L"-debug", L""} });
		});
}
