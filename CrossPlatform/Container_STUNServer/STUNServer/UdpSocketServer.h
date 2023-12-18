#pragma once
#include "Platform.h"
#include <asio.hpp>
#include <vector>
#include "UserConnection.h"
#include <iostream>

using namespace  asio::ip;

class UdpSocketServer {
public:
	UdpSocketServer();
	void StartListening();
private:
	void StartAcception();

	void HandleRead(UserConnection::pointer connection, const asio::error_code& error, size_t bytes_transferred);

private:
	asio::io_context context;
	udp::socket socket;
	std::string test;
	//std::vector <std::shared_ptr<UserConnection>> connections;
	std::vector <udp::endpoint> endpoints;
	udp::endpoint currentEndpoint;
	
};

