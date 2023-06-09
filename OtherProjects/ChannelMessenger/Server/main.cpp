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

void main() {
	try {
		channelServer.Create(L"\\\\.\\pipe\\$channelClientServer$",
			[](Channel<Messages>::ReadFunc Read, Channel<Messages>::WriteFunc Write) {
				auto reply = Read();
				switch (reply.type) {
				case Messages::Connect: {
					break;
				}
				case Messages::HelloFromClient: {
					BEEP(1000, 150);
					BEEP(1000, 150);
					break;
				}
				}
				return true;
			});

	}
	catch (PipeError err) {
		assert(false && "---> server pipe error");
	}

	while (true) {
		Sleep(10);
	};
	return;
}