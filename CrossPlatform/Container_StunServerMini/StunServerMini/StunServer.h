#pragma once
#include "Platform.h"
#include "PlatformLogger.h"
#include "StunServerModels.h"
#include "SocketBaseUdp.h"
#include <unordered_map>

struct UserExchangeInfo{
    std::unique_ptr<N::Server::UserAddress> userAddress;
    std::function<void(std::unique_ptr<N::Server::UserAddress>)> fetchCallback;
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
            auto serializedMessage = std::make_unique<std::vector<uint8_t>>(N::SerializeCustomMessage("Hello from Mini StunServer"));
            SendAsync(std::move(serializedMessage), boostAsio::buffer(*serializedMessage));
            break;
        }
        case N::QueryType::GetMyAddress: {
            printf("GetMyAddress \n");
            N::Client::Handshake handshakeData = *reinterpret_cast<N::Client::Handshake*>(receiveBuffer.data());

            auto clientAddress = std::make_unique<N::Server::UserAddress>();
            clientAddress->local = handshakeData.localAddress;
            clientAddress->global = { remoteEndpoint.address().to_v4().to_ulong(), remoteEndpoint.port() };
            SendAsync(std::move(clientAddress), boostAsio::buffer(clientAddress.get(), sizeof(N::Server::UserAddress)));
            break;
        }
        case N::QueryType::GetPartnerAddress: {
            printf("GetPartnerAddress [usersOnline = %d]\n", usersOnline.size());
            N::Client::AuthMessage authMessageData = *reinterpret_cast<N::Client::AuthMessage*>(receiveBuffer.data());

            auto currentClientAddress = std::make_unique<N::Server::UserAddress>();
            currentClientAddress->local = authMessageData.localAddress;
            currentClientAddress->global = { remoteEndpoint.address().to_v4().to_ulong(), remoteEndpoint.port() };

            auto currentClientExchangeInfo = std::make_unique<UserExchangeInfo>();
            currentClientExchangeInfo->userAddress = std::move(currentClientAddress);
            currentClientExchangeInfo->fetchCallback = [this, myRemoteEndpoint = this->remoteEndpoint](std::unique_ptr<N::Server::UserAddress> partnerAddress) {
                SendAsync(std::move(partnerAddress), boostAsio::buffer(partnerAddress.get(), sizeof(N::Server::UserAddress)), myRemoteEndpoint);
            };
            
            // Put requester to usersOnline
            auto& currentClientExchangeInfoRef = usersOnline[authMessageData.localId];
            currentClientExchangeInfoRef = std::move(currentClientExchangeInfo);

            // Fetch partner from usersOnline
            auto it = usersOnline.find(authMessageData.parnterId);
            if (it != usersOnline.end()) {
                auto& partnerExchangeInfo = it->second;

                // Send current client (requester) address to partner endpoint 
                partnerExchangeInfo->fetchCallback(std::move(currentClientExchangeInfoRef->userAddress));
                std::this_thread::sleep_for(std::chrono::milliseconds{ 200 });

                // Send partner address to current client (requester) endpoint 
                SendAsync(std::move(partnerExchangeInfo->userAddress), boostAsio::buffer(partnerExchangeInfo->userAddress.get(), sizeof(N::Server::UserAddress)));
                
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
    std::unordered_map<int, std::unique_ptr<UserExchangeInfo>> usersOnline;
};