#include "UdpSocketServer.h"

UdpSocketServer::UdpSocketServer() : socket{context, udp::endpoint(udp::v4(), 53463) }{

	test.resize(120);
	asio::socket_base::reuse_address option(true);
	//acceptor.set_option(tcp::socket::reuse_address(true));
}

void UdpSocketServer::StartListening(){
	StartAcception();
	context.run();
}

void UdpSocketServer::StartAcception() {
	socket.async_receive_from(asio::buffer(test), currentEndpoint, [this](asio::error_code, std::size_t recived) {
		std::cout << "got message";
		endpoints.push_back(currentEndpoint);
		if (endpoints.size() % 2 == 0 && endpoints.size() != 0) {
			const auto& firstClient = endpoints[endpoints.size() - 1];
			const auto& secondClient = endpoints[endpoints.size() - 2];
			
			auto adressFirst = firstClient.address().to_string();
			auto portFirst = firstClient.port();
			auto messageToSecond = adressFirst + ";"s + std::to_string(portFirst);

			auto adressSecond = secondClient.address().to_string();
			auto portSecond = secondClient.port();
			auto messageToFirst = adressSecond + ";"s + std::to_string(portSecond);

			socket.send_to(asio::buffer(messageToSecond), secondClient);
			socket.send_to(asio::buffer(messageToFirst), firstClient);
		}

		StartAcception();
		});
}



void UdpSocketServer::HandleRead(UserConnection::pointer connection,const asio::error_code& error, size_t bytes_transferred) {
	std::cout << error.message();

	//std::string messageString{ asio::buffer_cast<const char*>(connection->OutgointMessage().data())};
	//if (messageString == "0\n"s) {
	//	connection->WritePartnerData(connections[0]->GetAuthData());
	//}
	//else {
	//	connection->WritePartnerData(connections[1]->GetAuthData());
	//}
}
