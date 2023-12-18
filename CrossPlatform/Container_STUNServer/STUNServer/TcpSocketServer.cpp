#include "TcpSocketServer.h"

TcpSocketServer::TcpSocketServer() : acceptor{context, tcp::endpoint(tcp::v4(), 53463) }, test(120) {

	asio::socket_base::reuse_address option(true);
	acceptor.set_option(tcp::socket::reuse_address(true));
}

void TcpSocketServer::StartListening(){
	StartAcception();
	context.run();
}

void TcpSocketServer::StartAcception() {
	auto connection = UserConnection::Create(context);
	connections.push_back(connection);

	acceptor.async_accept(connection->Socket(), std::bind(&TcpSocketServer::HandleAccept, this, connection, std::placeholders::_1));
}

void TcpSocketServer::HandleAccept(UserConnection::pointer newConnection, asio::error_code error) {
	newConnection->WriteIpData();
	if (connections.size() % 2 == 0 && connections.size() != 0) {
		auto firstClient = connections[connections.size() - 1];
		auto secondClinet = connections[connections.size() - 2];
		firstClient->GetAuthData([secondClinet](std::string message) {
			secondClinet->WritePartnerData(message);
			});

		secondClinet->GetAuthData([firstClient](std::string message) {
			firstClient->WritePartnerData(message);
			});

	}

	//asio::async_read_until(newConnection->Socket(), newConnection->OutgointMessage(), "\n",std::bind(&TcpSocketServer::HandleRead, this, newConnection, std::placeholders::_1, std::placeholders::_2));
	StartAcception();
}

void TcpSocketServer::HandleRead(UserConnection::pointer connection,const asio::error_code& error, size_t bytes_transferred) {
	std::cout << error.message();

	//std::string messageString{ asio::buffer_cast<const char*>(connection->OutgointMessage().data())};
	//if (messageString == "0\n"s) {
	//	connection->WritePartnerData(connections[0]->GetAuthData());
	//}
	//else {
	//	connection->WritePartnerData(connections[1]->GetAuthData());
	//}
}
