#include <MagicEnum/MagicEnum.h>
#include <Helpers/Logger.h>
#include <AMQP/AMQPMessager.h>
#include <format>

int main() {
	lg::DefaultLoggers::Init("D:\\AMQP_Client.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
	auto amqpMessager = AMQPMessager{ "teamrd.net", 5243 };
	
	amqpMessager.InitMessageHandler();
	//auto error = amqpMessager.Connect(
	//	"client_rdm",
	//	"8c9f6788c54a410a98c1d09c6e821c37",
	//	"rdmvhost",
	//	"666001",
	//	AMQP::passive);

	auto error = amqpMessager.ConnectAsConsumer(
		"client_rdm",
		"8c9f6788c54a410a98c1d09c6e821c37",
		"rdmvhost",
		"666001");

	if (error != AMQPMessager::Error::NoError) {
		LOG_ERROR_D("amqpMessager Connect error = {}", MagicEnum::ToString(error));
	}

	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
	return 0;
}