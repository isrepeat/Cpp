#pragma once
#include "Platform.h"
#include "PlatformLogger.h"
#include "StunServerModels.h"
#include "SocketBaseUdp.h"
#include <unordered_map>

struct UserExchangeInfo{
    N::Server::UserAddress userAddress;
    std::function<void(N::Server::UserAddress)> fetchCallback;
};

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
        //LOG_DEBUG_D("-- [client] query [{}:{}] = {}", clientIp, clientPort, magic_enum::enum_name(queryType));
        printf("-- [client] query [%s:%d] = ", clientIp.c_str(), clientPort);

        switch (queryType) {
        case N::QueryType::Hello: {
            printf("Hello \n");
            // remember serialized msg for async send
            sendBuffer = N::SerializeCustomMessage("Hello from Mini StunServer");
            SendAsync(std::move(boostAsio::buffer(sendBuffer)));
            break;
        }
        case N::QueryType::GetMyAddress: {
            printf("GetMyAddress \n");
            N::Client::Handshake handshakeData = *reinterpret_cast<N::Client::Handshake*>(receiveBuffer.data());
            N::Server::UserAddress clientAddress;
            clientAddress.local = handshakeData.localAddress;
            clientAddress.global = { remoteEndpoint.address().to_v4().to_ulong(), remoteEndpoint.port() };
            SendAsync(std::move(boostAsio::buffer(&clientAddress, sizeof(N::Server::UserAddress))));
            break;
        }
        case N::QueryType::GetPartnerAddress: {
            printf("GetPartnerAddress [usersOnline = %d]\n", usersOnline.size());
            N::Client::AuthMessage authMessageData = *reinterpret_cast<N::Client::AuthMessage*>(receiveBuffer.data());

            N::Server::UserAddress clientAddress;
            clientAddress.local = authMessageData.localAddress;
            clientAddress.global = { remoteEndpoint.address().to_v4().to_ulong(), remoteEndpoint.port() };

            UserExchangeInfo clientRequester;
            clientRequester.userAddress = clientAddress;
            clientRequester.fetchCallback = [this, myRemoteEndpoint = this->remoteEndpoint](N::Server::UserAddress partnerAddress) {
                SendAsync(std::move(boostAsio::buffer(&partnerAddress, sizeof(N::Server::UserAddress))), myRemoteEndpoint);
            };
            usersOnline[authMessageData.localId] = clientRequester;

            auto it = usersOnline.find(authMessageData.parnterId);
            if (it != usersOnline.end()) {
                it->second.fetchCallback(clientRequester.userAddress);
                std::this_thread::sleep_for(std::chrono::milliseconds{ 200 });
                SendAsync(std::move(boostAsio::buffer(&it->second.userAddress, sizeof(N::Server::UserAddress))));
                
                printf("delete users \n");
                usersOnline.erase(authMessageData.localId);
                usersOnline.erase(authMessageData.parnterId);
                printf("usersOnline = %d \n", usersOnline.size());
            }
            break;
        }
        default:
            LOG_ERROR_D("Query not handled");
        }

        StartReceive();
    }

private:
    std::vector<uint8_t> sendBuffer;
    std::unordered_map<int, UserExchangeInfo> usersOnline;
};