#include <Helpers/Channel.h>
#include <inttypes.h>
#include <cassert>
#include <thread>

// Capturer messages
enum class Messages {
	None,
	Connect,
	FrameConfigurationRequest,
	FrameConfigurationData,
	FrameRequest,
	FrameData,
	DesktopChanged,
	AdapterChanged,
	UnhandledException,
	UserInput,
	Stop,
};


Channel<Messages> channelClient;
std::atomic<bool> exitApp = false;

void main() {
	try {
		channelClient.SetLoggerHandler([](std::wstring msg) {
			wprintf(L"[ch log] %s \n", msg.c_str());
			});

		channelClient.SetInterruptHandler([] {
			printf("was interrupt handler \n");
			exitApp = true;
			});

		channelClient.Open(L"\\\\.\\pipe\\$channelCapturer$",
			[](Channel<Messages>::ReadFunc Read, Channel<Messages>::WriteFunc Write) {
				auto reply = Read();
				switch (reply.type) {
				case Messages::Connect: {
					printf("Connected with ChannelServer \n\n");
					Sleep(1000);
					Write({}, Messages::FrameConfigurationRequest);
					break;
				}

				case Messages::FrameConfigurationData:
					printf("--> got  \"FrameConfigurationData\" \n\n");
					printf("send \"FrameRequest\" ... \n");
					Sleep(1000);
					Write({ (uint8_t)0 }, Messages::FrameRequest);
					break;

				case Messages::FrameData:
					printf("--> got  \"FrameData\" (size = %" PRId64 ") \n\n", reply.readData.size());
					break;
				}
				return true;
			});

	}
	catch (PipeError err) {
		//assert(false && "---> client pipe error");
		printf("was pipe error = %d \n", (int)err);
		exitApp = true;
	}


	while (!exitApp) {
		Sleep(10);
	};


	printf("Exit client ...");
	Sleep(3000);
	return;
}