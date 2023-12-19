#pragma once
#include "Platform.h"
#ifdef __WINDOWS_OS__
#include <msquic.hpp>
#else
#include <msquic.h>
#endif
#include <string>
#include <mutex>
#include <vector>
#include "QuicUserStream.h"
#include "ProtobufSerializer.h"
#include "UserDataExchanger.h"
#include "JwtReader.h"
#include <atomic>

class QuicSocketServer;

struct StreamContext {
	std::uint64_t id;
	QuicSocketServer* server;
};


class QuicSocketServer {

	struct UserMessage {
		std::string chunkedMessage;
		std::uint32_t messageSize = 0;
	};

public:
	QuicSocketServer(uint16_t port);
	~QuicSocketServer();
	void StartListening();

private:

	static _Function_class_(QUIC_LISTENER_CALLBACK) QUIC_STATUS ServerListenerCallback(HQUIC listener, void* context, QUIC_LISTENER_EVENT* event);
	static _Function_class_(QUIC_CONNECTION_CALLBACK) QUIC_STATUS ServerConnectionCallback(HQUIC connection, void* context, QUIC_CONNECTION_EVENT* event);
	static _Function_class_(QUIC_STREAM_CALLBACK) QUIC_STATUS ServerStreamCallback(HQUIC stream, void* context, QUIC_STREAM_EVENT* event);

	void SendMessageToClient(std::string message, HQUIC stream);

	void HandleUserMessage();

	UserAddress GetGlobalAddress(HQUIC connection);
private:
	HQUIC configuration;
	uint16_t listeningPort;
	HQUIC listener = nullptr;


	std::mutex usersMutex;

	const QUIC_BUFFER alpn = { sizeof("rdp") - 1, (uint8_t*)"rdp" };

	//std::string chunkedMessage;
	//std::atomic<uint32_t> messageSize = 0;
	std::unordered_map<uint64_t, UserMessage> usersMessages;

	std::atomic<int> activeConnections = 0;

	ProtobufSerializer serializer;
	UserDataExchanger users;
	JwtReader jwtReader{ "s5v9y$B?E(H+MbQeThWmZq4t7w!z%C*F)J@NcRfUjXn2r5u8x/A?D(G+KaPdSgVk","REMOTE_DESKTOP_MANAGER_SERVER" };
};		

