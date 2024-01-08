#pragma once
#include "Platform.h"
#include <Logger/Logger.h>
#include <string>

namespace boostAsio = __BOOST_NS::asio;
namespace boostSystem = __BOOST_SYSTEM_NS;

class SocketBaseUdp {
protected:
    SocketBaseUdp()
        : socket{ ioContext }
    {
        LOG_FUNCTION_ENTER("SocketBaseUdp()");
    }
    SocketBaseUdp(uint16_t port)
        : socket{ ioContext, boostAsio::ip::udp::endpoint(boostAsio::ip::udp::v4(), port) } // binded
    {
        LOG_FUNCTION_ENTER("SocketBaseUdp(port = {})", port);
    }
    SocketBaseUdp(std::string address, uint16_t port)
        : socket{ ioContext, boostAsio::ip::udp::endpoint(boostAsio::ip::address::from_string(address), port) } // binded
    {
        LOG_FUNCTION_ENTER("SocketBaseUdp(address = {}, port = {})", address, port);
    }

    void StartReceive(int maxBufferSize = 1024) {
        LOG_FUNCTION_ENTER("StartReceive()");
        receiveBuffer.clear();
        receiveBuffer.resize(maxBufferSize);

        socket.async_receive_from(
            boostAsio::buffer(receiveBuffer), remoteEndpoint,
            std::bind(&SocketBaseUdp::HandleReceive, this,
                std::placeholders::_1,
                std::placeholders::_2));
    }

    virtual void HandleReceive(const boostSystem::error_code& error, std::size_t bytesTransferred) = 0;

    virtual void SendAsync(boostAsio::mutable_buffers_1 buffer) {
        socket.async_send_to(buffer, remoteEndpoint,
            [this](const boostSystem::error_code& error, std::size_t bytesTransferred) {
                if (error) {
                    LOG_ERROR_D("socket error [async_send_to(...)] = {}", error.message());
                }
            });
    }

public:
    virtual void Stop() {
        LOG_FUNCTION_ENTER("Stop()");
        try {
            ioContext.stop();

            if (socket.is_open()) {
                LOG_DEBUG_D("close socket");
                socket.close();
            }
        }
        catch (std::exception& ex) {
            LOG_ERROR_D("Catch exception = {}", ex.what());
        }
    }

    ~SocketBaseUdp() {
        LOG_FUNCTION_ENTER("~SocketBaseUdp()");
        Stop();
    }

protected:
    boostAsio::io_context ioContext;
    boostAsio::ip::udp::socket socket;
    boostAsio::ip::udp::endpoint remoteEndpoint;
    //std::vector<uint8_t> sendBuffer;
    std::vector<uint8_t> receiveBuffer;
};
