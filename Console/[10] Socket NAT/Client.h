#pragma once
#include <Helpers/Logger.h>
#include "Common.h"

// Based from
// https://stackoverflow.com/questions/44273599/udp-communication-using-c-boost-asio

class Client {
public:
    Client()
        : socket{ io_service }
    {
    }

    void handle_receive(const boost::system::error_code& error, size_t bytes_transferred) {
        if (error) {
            LOG_ERROR_D("Receive failed: {}", error.message());
            return;
        }
        LOG_DEBUG_D("Received: {} (error = {})", std::string(recv_buffer.begin(), recv_buffer.begin() + bytes_transferred), error.message());

        if (--count > 0) {
            LOG_DEBUG_D("Count: {}\n", count);
            wait();
        }
    }

    void wait() {
        socket.async_receive(boost::asio::buffer(recv_buffer),
            boost::bind(&Client::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    void Receiver() {
        socket.open(udp::v4());
        socket.bind(udp::endpoint(address::from_string(IPADDRESS), UDP_PORT));

        wait();

        LOG_DEBUG_D("Receiving\n");
        io_service.run();
        LOG_DEBUG_D("Receiver exit\n");
    }

private:
    boost::asio::io_service io_service;
    boost::asio::ip::udp::socket socket;
    boost::array<char, 1024> recv_buffer;
    int count = 3;
};