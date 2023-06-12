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

void main() {
	try {
		channelClient.Open(L"\\\\.\\pipe\\$channelClientServer$",
			[](Channel<Messages>::ReadFunc Read, Channel<Messages>::WriteFunc Write) {
				auto reply = Read();
				switch (reply.type) {
				case Messages::Connect: {
					Sleep(3000);
					Write({}, Messages::HelloFromClient);
					BEEP(500, 500);
					break;
				}
				}
				return true;
			});

	}
	catch (PipeError err) {
		assert(false && "---> client pipe error");
	}


	channelClient.WaitFinishSendingMessage(Messages::HelloFromClient);

	//auto th = std::thread([] {
	//	Sleep(5'000);
	//	channelClient.Write({}, Messages::Stop);
	//	channelClient.WaitFinishSendingMessage(Messages::Stop);
	//	int xxx = 9;
	//	});



	while (true) {
		Sleep(10);
	};
	return;
}