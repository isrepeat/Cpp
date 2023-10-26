#define LOG_FUNCTION_ENTER_READ(...)
#define LOG_DEBUG_READ(...)

#define LOG_FUNCTION_ENTER_WRITE(...) LOG_FUNCTION_ENTER(__VA_ARGS__)
#define LOG_DEBUG_WRITE(...) LOG_DEBUG(__VA_ARGS__)

#include <MagicEnum/MagicEnum.h>
#include <Helpers/Channel.h>
#include <Helpers/HLogger.h>
#include <cassert>
#include <thread>

//#pragma comment(linker, "/STACK:16777216") // 16 * 1024 * 1024 [16 MB]
//#pragma comment(linker, "/STACK:524288") // 512 * 1024 [0.5 MB]
//#pragma comment(linker, "/STACK:1024") // 1024 [1 KB]

enum class Messages {
	None,
	Connect,
	FrameRequest,
	FrameData,
	UserInput,
	Stop,
};

int g_iteration = 0;

Channel<Messages> channelServer;
std::atomic<bool> exitApp = false;

void main() {
	lg::DefaultLoggers::Init(H::ExePathW() + L"\\Logs\\ServerLogs.log");
	LOG_DEBUG("Start server");

	try {
		channelServer.SetInterruptHandler([] {
			//exitApp = true;
			});

		channelServer.Create(L"\\\\.\\pipe\\$channelClientServer$",
			[](Channel<Messages>::Msg_t message, Channel<Messages>::WriteFunc Write) {
				switch (message->type) {
				case Messages::Connect: {
					LOG_DEBUG("[Connect]");
					break;
				}
				case Messages::FrameRequest: {
					LOG_DEBUG("[FrameRequest]");
					Write({1,2,3,4,5,6,7}, Messages::FrameData);
					break;
				}
				case Messages::UserInput: {
					//LOG_DEBUG("[UserInput] action");
					break;
				}
				}
				return true;
			});

	}
	catch (PipeError err) {
		LOG_ERROR_D("pipe error = {}", magic_enum::enum_name(err));
		exitApp = true;
	}

	std::vector<uint8_t> message_out{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

	auto thFrameData = std::thread([&message_out] {
		//while (!exitApp) {
		//	if (channelServer.IsConnected()) {
		//		//std::unique_lock lk{ mx };
		//		//std::vector<uint8_t> message = message_out;
		//		//std::vector<uint8_t> message{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
		//		//std::vector<uint8_t> message{ 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 };
		//		//std::vector<uint8_t> message2{ 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 };
		//		//std::vector<uint8_t> message(100 * 1024 * 1024);
		//		//std::vector<uint8_t> message(1024 * 100);
		//		//for (int i = 0; i < message.size(); i++) {
		//		//	message[i] = i;
		//		//}

		//		int tmp1 = 1;
		//		int tmp2 = 1;
		//		int tmp3 = 1;
		//		//channelServer.Write({}, Messages::FrameData);
		//		//std::vector<uint8_t> tmpMessage = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
		//		//std::vector<uint8_t> tmpMessage2 = std::move(tmpMessage);
		//		//channelServer.Write(std::move(tmpMessage2), Messages::FrameData);

		//		//channelServer.Write(Channel<Messages>::WriteMessage{ Messages::FrameData, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15} });
		//		channelServer.Write({ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }, Messages::FrameData);

		//		//channelServer.Write(message, Messages::FrameData);
		//		//channelServer.Write({ message.begin(), message.end() }, Messages::FrameData);
		//		//channelServer.Write(std::move(std::vector<uint8_t>{ 1,2,3,4,5,6,7,8,9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 }), Messages::FrameData);
		//		//channelServer.Write(std::move(message), Messages::FrameData);
		//		//Sleep(1);
		//		int tmp4 = 1;
		//		int tmp5 = 1;
		//		g_iteration++;
		//	}
		//}
		});


	while (!exitApp) {
		Sleep(10);
	};

	if (thFrameData.joinable()) {
		thFrameData.join();
	}

	LOG_DEBUG("Exit server");
	//Sleep(3000);
	return;
}