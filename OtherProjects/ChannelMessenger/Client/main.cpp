#include <Helpers/Channel.h>
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
					Write({}, Messages::HelloFromClient);
					break;
				}
				}
				return true;
			});

	}
	catch (PipeError err) {
		int xxx = 9;
	}

	auto th = std::thread([] {
		Sleep(5'000);
		channelClient.Write({}, Messages::Stop);
		channelClient.WaitFinishSendingMessage(Messages::Stop);
		int xxx = 9;
		});



	while (true) {
		Sleep(10);
	};
	return;
}