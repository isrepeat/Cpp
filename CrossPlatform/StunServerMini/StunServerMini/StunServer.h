#pragma once
#include "Platform.h"
#include "PlatformLogger.h"
#include "StunServerModels.h"
#include "SocketBaseUdp.h"


class StunServer : public SocketBaseUdp {
public:
    StunServer(uint16_t port)
        : SocketBaseUdp(port)
    {
        LOG_FUNCTION_ENTER("StunServer(port = {})", port);
        StartReceive();
    }

    ~StunServer() {
        LOG_FUNCTION_ENTER("~StunServer()");
    }

    // Block thread
    void StartListening() {
        LOG_FUNCTION_SCOPE("StartListening()");
        ioContext.run();
    }

private:
    // Handle only structs that inherited from N::Query
    void HandleReceive(const boostSystem::error_code& error, std::size_t bytesTransferred) override {
        LOG_FUNCTION_ENTER("HandleReceive(error, bytesTransferred)");
        if (error) {
            LOG_ERROR_D("socket error [HandleReceive(...)] = {}", error.message());
        }

        std::string clientIp = remoteEndpoint.address().to_string();
        uint16_t clientPort = remoteEndpoint.port();

        N::QueryType queryType = *reinterpret_cast<N::QueryType*>(receiveBuffer.data());
        LOG_DEBUG_D("-- [client] query [{}:{}] = {}", clientIp, clientPort, magic_enum::enum_name(queryType));

        switch (queryType) {
        case N::QueryType::GetMyAddress: {
            N::Client::Handshake handshakeData = *reinterpret_cast<N::Client::Handshake*>(receiveBuffer.data());
            N::Server::UserAddress clientAddress;
            clientAddress.local = handshakeData.localAddress;
            clientAddress.global = { remoteEndpoint.address().to_v4().to_ulong(), remoteEndpoint.port() };
            SendAsync(std::move(boostAsio::buffer(&clientAddress, sizeof(N::Server::UserAddress))));
            break;
        }
        default:
            LOG_ERROR_D("Query not handled");
        }

        StartReceive();
    }
};