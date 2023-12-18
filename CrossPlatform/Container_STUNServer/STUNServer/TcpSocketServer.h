#pragma once
#include "Platform.h"
#include <asio.hpp>
#include <vector>
#include "UserConnection.h"
#include <iostream>

using namespace  asio::ip;

class TcpSocketServer {
public:
	TcpSocketServer();
	void StartListening();
private:
	void StartAcception();
	void HandleAccept(UserConnection::pointer newConnection, asio::error_code error);

	void HandleRead(UserConnection::pointer connection, const asio::error_code& error, size_t bytes_transferred);

private:
	asio::io_context context;
	tcp::acceptor acceptor;
	std::vector<char> test;
	std::vector <std::shared_ptr<UserConnection>> connections;
};

