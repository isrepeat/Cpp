#include <MagicEnum/MagicEnum.h>
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

Channel<Messages> channelClient;
std::atomic<bool> exitApp = false;

void main() {
	try {
		channelClient.SetInterruptHandler([] {
			exitApp = true;
			});

		channelClient.Open(L"\\\\.\\pipe\\$channelClientServer$",
			[](Channel<Messages>::ReadFunc Read, Channel<Messages>::WriteFunc Write) {
				auto reply = Read();
				switch (reply.type) {
				case Messages::Connect: {
					printf("Connected with ChannelServer \n\n");
					printf("send \"HelloFromClient\" ... \n");
					Sleep(1000);
					Write({}, Messages::HelloFromClient);
					break;
				}
				case Messages::HelloFromServer: {
					printf("---> got \"HelloFromServer\" \n\n");
					printf("send \"UserRequest\" ... \n");
					Sleep(1000);
					Write({}, Messages::UserRequest);
					break;
				}
				case Messages::UserData: {
					printf("---> got \"UserData\" \n\n");
					//printf("send \"Stop\" ... \n");
					printf("break connection ... \n");
					Sleep(1000);
					return false;
				}
				}
				return true;
			});

	}
	catch (PipeError err) {
		LOG_ERROR_D("pipe error = {}", magic_enum::enum_name(err));
		exitApp = true;
	}


	//channelClient.WaitFinishSendingMessage(Messages::HelloFromClient);

	while (!exitApp) {
		Sleep(10);
	};


	printf("Exit client ...");
	Sleep(3000);
	return;
}