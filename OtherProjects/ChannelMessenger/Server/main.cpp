#include <Helpers/Channel.h>
#include <cassert>
#include <thread>

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

void main() {
	try {

		channelServer.SetLoggerHandler([] (std::wstring msg) {
			wprintf(L"[ch log] %s \n", msg.c_str());
			});

		channelServer.SetInterruptHandler([] {
			printf("was interrupt handler \n");
			//exitApp = true;
			});

		channelServer.CreateForAdmin(L"\\\\.\\pipe\\$channelClientServer$",
			[](Channel<Messages>::ReadFunc Read, Channel<Messages>::WriteFunc Write) {
				auto reply = Read();
				switch (reply.type) {
				case Messages::Connect: {
					printf("Connected with ChannelClient \n\n");
					break;
				}
				case Messages::HelloFromClient: {
					printf("---> got \"HelloFromClient\" \n\n");
					printf("send \"HelloFromServer\" ... \n");
					Sleep(1000);
					Write({}, Messages::HelloFromServer);
					break;
				}
				case Messages::UserRequest: {
					printf("---> got \"UserRequest\" \n\n");
					printf("send \"UserData\" ... \n");
					Sleep(1000);
					Write({}, Messages::UserData);
					break;
				}
				}
				return true;
			});

	}
	catch (PipeError err) {
		//assert(false && "---> server pipe error");
		printf("was pipe error = %d \n", (int)err);
		exitApp = true;
	}

	while (!exitApp) {
		Sleep(10);
	};

	printf("Exit server ...");
	Sleep(3000);
	return;
}