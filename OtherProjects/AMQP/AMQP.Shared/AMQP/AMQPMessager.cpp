#include "AMQPMessager.h"
#include <JsonParser/JsonParser.h>
#include <Helpers/Logger.h>
#include <Helpers/Action.h>
#include <Helpers/Thread.h>

#define QUEUE_EXPIRATION 70'000


//namespace AMQP {
//	struct BaseMessage {
//		int type;
//
//		JS_OBJECT(
//			JS_MEMBER_ALIASES(type, "Type")
//		);
//	};
//
//	struct ConnectionTargetInfoMessage : BaseMessage {
//		Device::ConnectionTargetInfo data;
//
//		JS_OBJECT_WITH_SUPER(
//			JS_SUPER(BaseMessage),
//			JS_MEMBER_ALIASES(data, "Data")
//		);
//	};
//}


AMQPMessager::AMQPMessager(std::string host, uint16_t port)
	: host{ host }
	, port{ port }
	, threadExceptionCallback{ std::bind(&AMQPMessager::ThreadExceptionCallback, this) }
{
	LOG_FUNCTION_ENTER("AMQPMessager()");
	CreateMessageHandlers();
}

AMQPMessager::~AMQPMessager() {
	LOG_FUNCTION_SCOPE("~AMQPMessager()");
	NotifyAboutStop();
	WaitingFinishThreads();
}


/* ------------------------------------ */
/*			 IThread methods			*/
/* ------------------------------------ */
void AMQPMessager::NotifyAboutStop() {
	if (channel) {
		channel->cancel(tag);
		channel->close();
	}
	if (amqpHandler) {
		amqpHandler->Quit();
		amqpHandler->Close();
	}
	pingerCv.notify_one();
}

void AMQPMessager::WaitingFinishThreads() {
	if (pingerThread && pingerThread->joinable()) {
		pingerThread->join();
	}
	if (workerThread && workerThread->joinable()) {
		workerThread->join();
	}
}



void AMQPMessager::InitMessageHandler() {
	LOG_FUNCTION_ENTER("InitMessageHandler()");

	if (amqpHandler) {
		LOG_WARNING_D("amqpHandler already initialized, ignore");
		return;
	}
	CreateAMQPHandler();
}

AMQPMessager::Error AMQPMessager::Connect(std::string login, std::string password, std::string virtualHost, std::string machineId, int queueFlags) {
	LOG_FUNCTION_SCOPE("Connect(login, password, virtualHost, machineId = {})", machineId);

	if (!amqpHandler) {
		LOG_WARNING_D("You need init AMQPHandler before create channel and connection");
		return Error::AMQPHanlerNotInited;
	}

	if (channel) {
		LOG_WARNING_D("AMQP channel already connected");
		return Error::ChannelAlreadyConnected;
	}

	queueName = machineId;
	connection = std::make_unique<AMQP::Connection>(amqpHandler.get(), AMQP::Login{ login, password }, virtualHost);
	channel = std::make_unique<AMQP::Channel>(connection.get());

	auto args = AMQP::Table{};
	args["x-expires"] = QUEUE_EXPIRATION; // [ms]
	channel->declareQueue(queueName, queueFlags, args).onError([](const char* err) {
		LOG_DEBUG_D("channel queue error = {}", err);
		});

	channel->consume(queueName, tag, AMQP::noack).onReceived([this](const AMQP::Message& message, uint64_t deliveryTag, bool redelivered) {
		LOG_FUNCTION_ENTER("AMQPMessager->onReceived(message, ...)");

		auto messageData = std::string(message.body(), static_cast<int>(message.bodySize()));
		LOG_DEBUG_D("AMQPMessager message = {}", messageData);

		//AMQP::BaseMessage baseMessage;
		//if (!JS::ParseTo(messageData.constData(), messageData.size(), baseMessage))
		//	return;

		//auto handler = messageHandlers[static_cast<MessageType>(baseMessage.type)];
		//handler(messageData);
		});

	LOG_DEBUG_D("RabbitMQ inited");

	workerThread = std::make_unique<H::ThreadEx>(threadExceptionCallback, [this] {
		LOG_THREAD(L"AMQPMessager worker thread");
		amqpHandler->StartLoop();
		});

	pingerThread = std::make_unique<H::ThreadEx>(threadExceptionCallback, [this] {
		LOG_THREAD(L"AMQPMessager pinger thread");
		while (!amqpHandler->IsQuit()) {
			auto lock = std::unique_lock{ pingerMutex };
			pingerCv.wait_for(lock, std::chrono::seconds(30));
			connection->heartbeat();
		}
		});

	return Error::NoError;
}

AMQPMessager::Error AMQPMessager::ConnectAsConsumer(std::string login, std::string password, std::string virtualHost, std::string machineId) {
	LOG_FUNCTION_SCOPE("Connect(login, password, virtualHost, machineId = {})", machineId);

	if (!amqpHandler) {
		LOG_WARNING_D("You need init AMQPHandler before create channel and connection");
		return Error::AMQPHanlerNotInited;
	}

	if (channel) {
		LOG_WARNING_D("AMQP channel already connected");
		return Error::ChannelAlreadyConnected;
	}

	queueName = machineId;
	connection = std::make_unique<AMQP::Connection>(amqpHandler.get(), AMQP::Login{ login, password }, virtualHost);
	channel = std::make_unique<AMQP::Channel>(connection.get());

	auto args = AMQP::Table{};
	args["x-expires"] = QUEUE_EXPIRATION; // [ms]
	channel->declareQueue(queueName, AMQP::passive, args).onError([](const char* err) {
		LOG_DEBUG_D("channel queue error = {}", err);
		});

	channel->consume(queueName, tag, AMQP::noack)
		.onReceived(
			[this](const AMQP::Message& message, uint64_t deliveryTag, bool redelivered) {
				LOG_FUNCTION_ENTER("AMQPMessager->onReceived(message, ...)");

				auto messageData = std::string(message.body(), static_cast<int>(message.bodySize()));
				LOG_DEBUG_D("AMQPMessager message = {}", messageData);

				//AMQP::BaseMessage baseMessage;
				//if (!JS::ParseTo(messageData.constData(), messageData.size(), baseMessage))
				//	return;

				//auto handler = messageHandlers[static_cast<MessageType>(baseMessage.type)];
				//handler(messageData);
		})
		.onSuccess(
			[this](const std::string& consumertag) {
				LOG_FUNCTION_ENTER("AMQPMessager->onSuccess(consumertag = {})", consumertag);
			})
		.onError(
			[this](const char* errorMessage) {
				LOG_FUNCTION_ENTER("AMQPMessager->onError(errorMessage = {})", errorMessage);
			});

	LOG_DEBUG_D("RabbitMQ inited");

	workerThread = std::make_unique<H::ThreadEx>(threadExceptionCallback, [this] {
		LOG_THREAD(L"AMQPMessager worker thread");
		amqpHandler->StartLoop();
		});

	pingerThread = std::make_unique<H::ThreadEx>(threadExceptionCallback, [this] {
		LOG_THREAD(L"AMQPMessager pinger thread");
		while (!amqpHandler->IsQuit()) {
			auto lock = std::unique_lock{ pingerMutex };
			pingerCv.wait_for(lock, std::chrono::seconds(30));
			connection->heartbeat();
		}
		});

	return Error::NoError;
}

AMQPMessager::Error AMQPMessager::ConnectAsPublisher(std::string login, std::string password, std::string virtualHost, std::string machineId) {
	LOG_FUNCTION_SCOPE("Connect(login, password, virtualHost, machineId = {})", machineId);

	if (!amqpHandler) {
		LOG_WARNING_D("You need init AMQPHandler before create channel and connection");
		return Error::AMQPHanlerNotInited;
	}

	if (channel) {
		LOG_WARNING_D("AMQP channel already connected");
		return Error::ChannelAlreadyConnected;
	}

	queueName = machineId;
	connection = std::make_unique<AMQP::Connection>(amqpHandler.get(), AMQP::Login{ login, password }, virtualHost);
	channel = std::make_unique<AMQP::Channel>(connection.get());

	auto args = AMQP::Table{};
	args["x-expires"] = QUEUE_EXPIRATION; // [ms]
	channel->declareQueue(queueName, 0, args).onError([](const char* err) {
		LOG_DEBUG_D("channel queue error = {}", err);
		});

	//channel->consume(queueName, tag, AMQP::noack).onReceived([this](const AMQP::Message& message, uint64_t deliveryTag, bool redelivered) {
	//	LOG_FUNCTION_ENTER("AMQPMessager->onReceived(message, ...)");

	//	auto messageData = std::string(message.body(), static_cast<int>(message.bodySize()));
	//	LOG_DEBUG_D("AMQPMessager message = {}", messageData);

	//	//AMQP::BaseMessage baseMessage;
	//	//if (!JS::ParseTo(messageData.constData(), messageData.size(), baseMessage))
	//	//	return;

	//	//auto handler = messageHandlers[static_cast<MessageType>(baseMessage.type)];
	//	//handler(messageData);
	//	});

	LOG_DEBUG_D("RabbitMQ inited");

	workerThread = std::make_unique<H::ThreadEx>(threadExceptionCallback, [this] {
		LOG_THREAD(L"AMQPMessager worker thread");
		amqpHandler->StartLoop();
		});

	pingerThread = std::make_unique<H::ThreadEx>(threadExceptionCallback, [this] {
		LOG_THREAD(L"AMQPMessager pinger thread");
		while (!amqpHandler->IsQuit()) {
			auto lock = std::unique_lock{ pingerMutex };
			pingerCv.wait_for(lock, std::chrono::seconds(30));
			connection->heartbeat();
		}
		});

	return Error::NoError;
}

void AMQPMessager::Send(std::string msg) {
	channel->publish("", queueName, msg.c_str());
}

void AMQPMessager::RecreateChannelConnection() {
	LOG_FUNCTION_SCOPE("RecreateChannelConnection()");

	NotifyAboutStop();
	LOG_DEBUG_D("channel closed");

	WaitingFinishThreads();
	LOG_DEBUG_D("threads finished");

	// Ensure that Dtors finish all internal work if need
	channel = nullptr;
	connection = nullptr;
	LOG_DEBUG_D("channel and connection reset");

	CreateAMQPHandler();
	LOG_DEBUG_D("AMQP handler recreated");
}

// NOTE: Call it in background thread because it may take a long time 
void AMQPMessager::CreateAMQPHandler() {
	LOG_FUNCTION_ENTER("CreateAMQPHandler()");

	H::PerformActionWithAttempts(5, [this] {
		try {
			amqpHandler = nullptr; // first call Dtor
			amqpHandler = std::make_unique<AMQPHandler>(host, port);
			H::BreakAction;
		}
		catch (asio::error_code& err) {
			// try create amqpHandler again ...
			LOG_ERROR_D("Catch asio::error_code = {}", err.message());
		}
		});
}

void AMQPMessager::CreateMessageHandlers() {
	//messageHandlers[MessageType::ConnectionRequest] = [this](const QByteArray& response) {
	//	LOG_DEBUG("Connection request message");

	//	AMQP::ConnectionTargetInfoMessage connectionTargetInfoMessage;
	//	if (!JS::ParseTo(response.constData(), response.size(), connectionTargetInfoMessage))
	//		return;

	//	emit ConnectionRequestMessage(connectionTargetInfoMessage.data);
	//};

	//messageHandlers[MessageType::ReadyToConnect] = [this](const QByteArray& response) {
	//	LOG_DEBUG("Ready to connect message");

	//	AMQP::ConnectionTargetInfoMessage connectionTargetInfoMessage;
	//	if (!JS::ParseTo(response.constData(), response.size(), connectionTargetInfoMessage))
	//		return;

	//	emit ClientReadyToConnectMessage(connectionTargetInfoMessage.data);
	//};

	//messageHandlers[MessageType::NatTraversalSucceeded] = [this](const QByteArray& response) {
	//	LOG_DEBUG("Partner passed nat");
	//	emit PartnerPassedNat();
	//};
}

void AMQPMessager::ThreadExceptionCallback() {
	LOG_FUNCTION_ENTER("ThreadExceptionCallback()");
	//emit WasChannelException();
}