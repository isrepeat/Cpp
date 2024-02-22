#include <MagicEnum/MagicEnum.h>
#include <Helpers/Logger.h>
#include <AMQP/AMQPMessager.h>
#include <format>

int main() {
	lg::DefaultLoggers::Init(H::ExePath() / "AMQPLogs\\AMQP_Server.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
	auto amqpMessager = AMQPMessager{ "teamrd.net", 5243 };
	
	amqpMessager.InitMessageHandler();
	//auto error = amqpMessager.Connect(
	//	"client_rdm",
	//	"8c9f6788c54a410a98c1d09c6e821c37",
	//	"rdmvhost",
	//	"666001");	
	
	//auto error = amqpMessager.Connect(
	auto error = amqpMessager.ConnectAsPublisher(
		"server_rdm",
		"b740a43fdce9440ebbedad5296150842",
		"rdmvhost",
		"666001");

	if (error != AMQPMessager::Error::NoError) {
		LOG_ERROR_D("amqpMessager Connect error = {}", MagicEnum::ToString(error));
	}

	int msgIdx = 0;
	while (true) {
		amqpMessager.Send(std::format("Hello from server [{}]", msgIdx++));
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	return 0;
}