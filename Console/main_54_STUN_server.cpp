// https://sciencesoftcode.wordpress.com/2018/12/11/boost-asio-network-and-low-level-i-o-programming/
#define _CRT_SECURE_NO_WARNINGS
#include <Helpers/Logger.h>
#include <MagicEnum/MagicEnum.h>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <string>
#include <ctime>

namespace {
    const std::filesystem::path publicFolder = H::GetKnownFolder(FOLDERID_LocalDocuments);
    const std::filesystem::path appLocalFolder = publicFolder / "StunServerMini";

    std::string make_daytime_string() {
        using namespace std; // For time_t, time and ctime;
        time_t now = time(0);
        return ctime(&now);
    }
}

#define SERVER_ADDR "192.168.0.101"
#define SERVER_PORT 13200

using boost::asio::ip::udp;
using boost::asio::ip::address;

namespace N {
    enum class QueryType {
        None,
        GetMyAddress,
        GetPartnerAddress,
    };

    enum class ResponseType {
        None,
        UserAddress,
    };

#pragma pack(push, 1)
    template<QueryType queryType>
    struct Query {
        const QueryType type = queryType;
    };

    template<ResponseType responseType>
    struct Response {
        const ResponseType type = responseType;
    };

    // Helpers:
    struct endpoint {
        uint32_t ip = 0;
        uint16_t port = 0;
    };

    
    // Client queries
    namespace Client {
        struct Handshake : Query<QueryType::GetMyAddress> {
            endpoint localAddress;
        };

        struct AuthMessage : Query<QueryType::GetPartnerAddress> {
            endpoint localAddress;
            int localId = 0;
            int parnterId = 0;
        };
    }


    // Server responses
    namespace Server {
        struct UserAddress : Response<ResponseType::UserAddress> {
            endpoint local;
            endpoint global;
        };
    }
#pragma pack(pop)
}


namespace H {
    boost::asio::ip::address GetLocalAddress(int attempts = 1) {
        LOG_FUNCTION_ENTER("GetLocalEndpoint()");
        using namespace boost;

        std::string googleDnsServerIp = "8.8.8.8";
        uint16_t googleDnsServerPort = 53;

        try {
            asio::io_context io_context;
            asio::ip::tcp::socket socket(io_context);
            socket.open(asio::ip::tcp::v4());
            socket.bind(asio::ip::tcp::endpoint{ asio::ip::tcp::v4(), 0 });

            asio::io_service ioService;
            asio::ip::tcp::resolver resolver(ioService);

            auto hostAddress = resolver.resolve(googleDnsServerIp, "")->endpoint().address().to_string();
            LOG_DEBUG_D("resolved hostAddress = {}", hostAddress);

            boost::system::error_code errCode;
            do {
                LOG_DEBUG_D("socket connect ...");
                socket.connect(asio::ip::tcp::endpoint{ asio::ip::address_v4::from_string(hostAddress), googleDnsServerPort }, errCode);

                if (errCode) {
                    LOG_ERROR_D("socket connect error = {}", errCode.message());

                    if (--attempts > 0) {
                        LOG_DEBUG_D("try connect again, ({} attempts left)");
                        std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
                    }
                    else {
                        LOG_DEBUG_D("Connection attempts ended");
                        break;
                    }
                }
            } while (errCode);


            auto localEndpoint = socket.local_endpoint();
            LOG_DEBUG_D("Local address = {}:{}", localEndpoint.address().to_string(), localEndpoint.port());
            return localEndpoint.address();
        }
        catch (std::exception& ex) {
            LOG_ERROR_D("Caught exception = {}", ex.what());
        }
        return {};
    }
}


class SocketBaseUdp {
protected:
    SocketBaseUdp()
        : socket{ ioContext }
    {
        LOG_FUNCTION_ENTER("SocketBaseUdp()");
    }
    SocketBaseUdp(uint16_t port)
        : socket{ ioContext, udp::endpoint(udp::v4(), port) } // binded
    {
        LOG_FUNCTION_ENTER("SocketBaseUdp(port = {})", port);
    }
    SocketBaseUdp(std::string address, uint16_t port)
        : socket{ ioContext, udp::endpoint(address::from_string(address), port) } // binded
    {
        LOG_FUNCTION_ENTER("SocketBaseUdp(address = {}, port = {})", address, port);
    }

    void StartReceive(int maxBufferSize = 1024) {
        LOG_FUNCTION_ENTER("StartReceive()");
        receiveBuffer.clear();
        receiveBuffer.resize(maxBufferSize);

        socket.async_receive_from(
            boost::asio::buffer(receiveBuffer), remoteEndpoint,
            boost::bind(&SocketBaseUdp::HandleReceive, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    virtual void HandleReceive(const boost::system::error_code& error, std::size_t bytesTransferred) = 0;

    virtual void SendAsync(boost::asio::mutable_buffers_1 buffer) {
        socket.async_send_to(buffer, remoteEndpoint,
            [this](const boost::system::error_code& error, std::size_t bytesTransferred) {
                if (error) {
                    LOG_ERROR_D("socket error [async_send_to(...)] = {}", error.message());
                }
            });
    }

public:
    virtual void Stop() {
        LOG_FUNCTION_ENTER("Stop()");
        try{
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
    boost::asio::io_context ioContext;
    udp::socket socket;
    udp::endpoint remoteEndpoint;
    //std::vector<uint8_t> sendBuffer;
    std::vector<uint8_t> receiveBuffer;
};


class StunServer : public SocketBaseUdp {
public:
    StunServer(uint16_t port)
        : SocketBaseUdp(SERVER_ADDR, port)
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
    void HandleReceive(const boost::system::error_code& error, std::size_t bytesTransferred) override {
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
            SendAsync(std::move(boost::asio::buffer(&clientAddress, sizeof(N::Server::UserAddress))));
            break;
        }
        
        //case N::Query::GetPartnerAddress: {
        //    //N::endpoint clientEndpoint = { remoteEndpoint.address().to_v4().to_ulong(), remoteEndpoint.port() };
        //    //boost::asio::buffer(&clientEndpoint, sizeof(N::endpoint));
        //    break;
        //}
        default:
            LOG_ERROR_D("Query not handled");
        }  

        StartReceive();
    }

private:

};



class UserClient : public SocketBaseUdp {
public:
    UserClient() {
        LOG_FUNCTION_ENTER("UserClient()");
    }

    ~UserClient() {
        LOG_FUNCTION_ENTER("~UserClient()");
        this->Stop();
    }

    void ConnectToServer(std::string ip, uint16_t port) {
        LOG_FUNCTION_ENTER("ConnectToServer(ip = {}, port = {})", ip, port);

        remoteEndpoint = udp::endpoint{ address::from_string(ip), port };
        
        // Run context after socket binded
        ioContextThread = std::thread([this] {
            LOG_THREAD(L"UserClient->ioContext thread");
            ioContext.run();
            });

        socket.open(udp::v4());
        socket.bind(udp::endpoint(udp::v4(), 0));

        StartReceive();

        auto localAddress = H::GetLocalAddress();
        auto localBindedPort = socket.local_endpoint().port();
        LOG_DEBUG_D("Local binded udp port = {}", localBindedPort);

        LOG_DEBUG_D("Handshake msg: localAddress = {}:{}", localAddress.to_v4().to_ulong(), localBindedPort);
        N::Client::Handshake hanshakeMsg;
        hanshakeMsg.localAddress = { localAddress.to_v4().to_uint(), localBindedPort };
        SendAsync(std::move(boost::asio::buffer(&hanshakeMsg, sizeof(N::Client::Handshake))));
    }

    void Stop() override {
        LOG_FUNCTION_SCOPE("Stop()");
        try {
            ioContext.stop();
            if (ioContextThread.joinable())
                ioContextThread.join();

            if (socket.is_open()) {
                LOG_DEBUG_D("close socket");
                socket.close();
            }
        }
        catch (std::exception& ex) {
            LOG_ERROR_D("Catch exception = {}", ex.what());
        }
    }

private:
    // Handle only structs that inherited from N::Response
    void HandleReceive(const boost::system::error_code& error, std::size_t bytesTransferred) override {
        LOG_FUNCTION_ENTER("HandleReceive(error, bytesTransferred)");
        if (error) {
            LOG_ERROR_D("socket error [HandleReceive(...)] = {}", error.message());
        }

        N::ResponseType responseType = *reinterpret_cast<N::ResponseType*>(receiveBuffer.data());
        LOG_DEBUG_D("-- [server] response = {}", magic_enum::enum_name(responseType));

        switch (responseType) {
        case N::ResponseType::UserAddress: {
            N::Server::UserAddress myAddress = *reinterpret_cast<N::Server::UserAddress*>(receiveBuffer.data());
            udp::endpoint myLocalEndpoint = udp::endpoint{ boost::asio::ip::address_v4{myAddress.local.ip}, myAddress.local.port };
            udp::endpoint myGlobalEndpoint = udp::endpoint{ boost::asio::ip::address_v4{myAddress.global.ip}, myAddress.global.port };
            LOG_DEBUG_D(
                "   My address:\n"
                "    * local = {}:{}\n"
                "    * global = {}:{}\n"
                , myLocalEndpoint.address().to_string(), myLocalEndpoint.port()
                , myGlobalEndpoint.address().to_string(), myGlobalEndpoint.port()
            );
            break;
        }

        default:
            LOG_ERROR_D("Query not handled");
        }

        StartReceive();
    }

private:
    std::thread ioContextThread;
};






int main() {
    lg::DefaultLoggers::Init(appLocalFolder / "StunServerMini.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
    LOG_DEBUG_D("StunServer start");


    auto thUserSender = std::thread([] {
        Sleep(1000);

        try {
            UserClient userClient;
            userClient.ConnectToServer(SERVER_ADDR, SERVER_PORT);
            Sleep(10000);
            LOG_DEBUG_D("Disconnect userClient");

            //udp::endpoint serverEndpoint = udp::endpoint(address::from_string(SERVER_ADDR), SERVER_PORT);
            //
            //boost::asio::io_context ioContextUser;
            //udp::socket socket(ioContextUser);

            //LOG_DEBUG_D("open socket, send msg ...");
            //socket.open(udp::v4());
            //socket.bind(udp::endpoint(udp::v4(), 0));

            //auto thUserIoContext = std::thread([&ioContextUser] {
            //    ioContextUser.run();
            //    LOG_DEBUG_D("ioContextUser stoped");
            //    });

            ////std::string receiveBuffer;
            //std::vector<uint8_t> receiveBuffer;
            //receiveBuffer.resize(100);

            //udp::endpoint remoteEndpoint;
            //socket.async_receive(boost::asio::buffer(receiveBuffer),
            //    [&receiveBuffer](const boost::system::error_code& error, std::size_t bytesTransferred) {
            //        LOG_FUNCTION_ENTER("Receive_lambda(error, bytesTransferred)");
            //        if (error) {
            //            LOG_ERROR_D("socket error [Receive_lambda(...)] = {}", error.message());
            //        }

            //        N::endpoint* clientEndpoint = reinterpret_cast<N::endpoint*>(receiveBuffer.data());
            //        LOG_DEBUG_D("-- receive message from [server] = {}", 1);
            //    });

            //socket.send_to(boost::asio::buffer("Hello"), serverEndpoint);
            //Sleep(3000);

            //LOG_DEBUG_D("stop ioContextUser");
            //ioContextUser.stop();

            //if (thUserIoContext.joinable())
            //    thUserIoContext.join();

            //LOG_DEBUG_D("close socket");
            //socket.close();
        }
        catch (std::exception& ex) {
            LOG_ERROR_D("Catch exception = {}", ex.what());
        }
        });


    try {
        StunServer stunServer(SERVER_PORT);
        stunServer.StartListening();
    }
    catch (std::exception& ex) {
        LOG_ERROR_D("Catch exception = {}", ex.what());
    }

    if (thUserSender.joinable())
        thUserSender.join();

    LOG_DEBUG_D("StunServer exit");
    return 0;
}