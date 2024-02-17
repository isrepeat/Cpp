#pragma once
#include <Helpers/Thread.h>
//#include "HttpClientModels.h"
#include "AMQPHandler.h"
#include <string>
#include <memory>

class AMQPMessager : public H::IThread {
public:
	enum class Error {
		NoError,
		AMQPHanlerNotInited,
		ChannelAlreadyConnected,
	};

	AMQPMessager(std::string host, uint16_t port);
	~AMQPMessager();

	// IThread
	void NotifyAboutStop() override;
	void WaitingFinishThreads() override;

	void InitMessageHandler();
	Error Connect(std::string login, std::string password, std::string virtualHost, std::string machineId, int queueFlags = 0);

	void Send(std::string msg);

	void RecreateChannelConnection();

	//signals:
	//	void ConnectionRequestMessage(Device::ConnectionTargetInfo connectionInfo);
	//	void ClientReadyToConnectMessage(Device::ConnectionTargetInfo connectionInfo);
	//	void PartnerPassedNat();

	void WasChannelException();


private:
	enum class MessageType {
		ConnectionRequest = 1,
		ReadyToConnect,
		NatTraversalSucceeded,
	};

	void CreateAMQPHandler();
	void CreateMessageHandlers();

	void ThreadExceptionCallback();

private:
	const std::string tag = "rdpClient";
	const std::string host;
	const std::uint16_t port;
	const std::function<void()> threadExceptionCallback;

	std::mutex mx;
	std::string queueName;
	std::unique_ptr<AMQPHandler> amqpHandler;
	std::unique_ptr<AMQP::Connection> connection;
	std::unique_ptr<AMQP::Channel> channel;

	std::unique_ptr<H::ThreadEx> workerThread;

	std::mutex pingerMutex;
	std::condition_variable pingerCv;
	std::unique_ptr<H::ThreadEx> pingerThread;
};