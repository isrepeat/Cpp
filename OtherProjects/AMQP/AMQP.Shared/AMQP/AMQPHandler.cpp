#include "AMQPHandler.h"
#include <Helpers/Logger.h>

#include <iostream>
#include <cassert>
#include <cstring>
#include <chrono>
#include <thread>
#include <vector>

AMQPHandler::AMQPHandler(const std::string& host, uint16_t port)
	: inputBuffer(AMQPHandler::BUFFER_SIZE)
	, outBuffer(AMQPHandler::BUFFER_SIZE)
	, tmpBuff(AMQPHandler::TEMP_BUFFER_SIZE)
	, socket{ context }
{
	LOG_FUNCTION_SCOPE("AMQPHandler(host = {}, port = {})", host, port);

	socket.open(asio::ip::tcp::v4());
	socket.bind(asio::ip::tcp::endpoint{ asio::ip::tcp::v4(), 0 });
	auto option = asio::ip::tcp::socket::keep_alive{ true };
	socket.set_option(option);

	asio::io_service ioService;
	asio::ip::tcp::resolver resolver(ioService);

	asio::error_code errCode;
	auto resolveResult = resolver.resolve(host, "", errCode);
	if (errCode) {
		LOG_ERROR_D("resolver error = {}", errCode.message());
		throw errCode;
	}

	auto hostAddress = resolveResult->endpoint().address().to_string();
	LOG_DEBUG_D("resolved hostAddress = {}", hostAddress);


	LOG_DEBUG_D("socket connect ...");
	socket.connect(asio::ip::tcp::endpoint{ asio::ip::address_v4::from_string(hostAddress), port }, errCode);

	if (errCode) {
		LOG_ERROR_D("soket connect error = {}", errCode.message());
		throw errCode;
	}

	std::string localIp = socket.local_endpoint().address().to_string();
	LOG_DEBUG_D("socket local ip = {}", localIp);
}

AMQPHandler::~AMQPHandler() {
	LOG_FUNCTION_SCOPE("~AMQPHandler()");
	Close();
}


void AMQPHandler::StartLoop() {
	try {
		while (!quit) {
			// Decode incomming RabbitMQ data from socket
			if (socket.available() > 0) {
				size_t avail = socket.available();

				if (tmpBuff.size() < avail) {
					tmpBuff.clear();
					tmpBuff.resize(avail, 0);
				}

				socket.receive(asio::buffer(tmpBuff, avail));
				inputBuffer.write(tmpBuff.data(), avail);
				tmpBuff.clear();
			}

			if (connection && inputBuffer.available()) {
				size_t count = connection->parse(inputBuffer.data(), inputBuffer.available());
				if (count == inputBuffer.available()) {
					inputBuffer.drain();
				}
				else if (count > 0) {
					inputBuffer.shl(count);
				}
			}

			SendDataFromBuffer();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		SendDataFromBuffer();
	}
	catch (std::exception ex) {
		if (!quit) {
			LOG_ERROR_D("Catch std::exception = {}", ex.what());
			throw;
		}
	}
}

void AMQPHandler::Quit() {
	quit = true;
}

void AMQPHandler::AMQPHandler::Close() {
	socket.close();
}


void AMQPHandler::onConnected(AMQP::Connection* connection) {
	LOG_FUNCTION_ENTER("onConnected(connection)");
	connected = true;
}

void AMQPHandler::onData(AMQP::Connection* connection, const char* data, size_t size) {
	this->connection = connection;

	const size_t writen = outBuffer.write(data, size);
	if (writen != size) {
		SendDataFromBuffer();
		outBuffer.write(data + writen, size - writen);
	}
}

void AMQPHandler::onError(AMQP::Connection* connection, const char* message) {
	LOG_FUNCTION_ENTER("onError(connection, message)");
	LOG_ERROR_D("AMQP error = {}", message);
}

void AMQPHandler::onClosed(AMQP::Connection* connection) {
	LOG_FUNCTION_ENTER("onClosed(connection)");
	quit = true;
}

std::uint16_t AMQPHandler::onNegotiate(AMQP::Connection* connection, std::uint16_t interval) {
	LOG_FUNCTION_ENTER("onNegotiate()");
	return 0;
}


bool AMQPHandler::IsQuit() const {
	return quit;
}

bool AMQPHandler::IsConnected() const {
	return connected;
}

bool AMQPHandler::IsSocketOpen() const {
	return socket.is_open();
}

void AMQPHandler::SendDataFromBuffer() {
	if (outBuffer.available()) {
		socket.send(asio::buffer(const_cast<char*>(outBuffer.data()), outBuffer.available()));
		outBuffer.drain();
	}
}