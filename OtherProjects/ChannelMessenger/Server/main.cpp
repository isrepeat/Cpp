#include <MagicEnum/MagicEnum.h>
#include <Helpers/Channel.h>
#include <Helpers/HLogger.h>
#include <cassert>
#include <thread>

enum class Messages {
	None,
	Connect,
	FrameRequest,
	FrameData,
	UserInput,
	Stop,
};

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
			[](Channel<Messages>::ReadFunc Read, Channel<Messages>::WriteFunc Write) {
				auto reply = Read();
				switch (reply.type) {
				case Messages::Connect: {
					LOG_DEBUG("[Connect] Connected with ChannelClient");
					break;
				}
				case Messages::FrameRequest: {
					////LOG_DEBUG("[FrameRequest]");
					//std::vector<uint8_t> message(300*1024); // 300 KB
					//for (int i = 0; i < message.size(); i++) {
					//	message[i] = i;
					//}
					//Write({ message.begin(), message.end() }, Messages::FrameData);
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


	auto thFrameData = std::thread([] {
		while (!exitApp) {
			if (channelServer.IsConnected()) {
				//std::unique_lock lk{ mx };
				std::vector<uint8_t> message(300 * 1024); // 300 KB
				for (int i = 0; i < message.size(); i++) {
					message[i] = i;
				}
				channelServer.Write({ message.begin(), message.end() }, Messages::FrameData);
			}
		}
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