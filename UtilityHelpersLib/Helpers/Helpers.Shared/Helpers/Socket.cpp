#pragma once
#include "BoostAsioSafe.h" // must be first to have no errors with boost/asio and winsock

#include "Socket.h"
#ifdef BOOST_SUPPORTED
#include "Logger.h"

using namespace boost;

namespace HELPERS_NS {
    std::string GetLocalIp(int attempts) {
        LOG_FUNCTION_ENTER("GetLocalIp()");

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
                    LOG_ERROR_D("soket connect error = {}", errCode.message());

                    if (--attempts > 0) {
                        LOG_DEBUG_D("try connect again, ({} attempts left)", attempts);
                        std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
                    }
                    else {
                        LOG_DEBUG_D("Connection attempts ended");
                        break;
                    }
                }
            } while (errCode);

            std::string localIp = socket.local_endpoint().address().to_string();
            LOG_DEBUG_D("Local Ip = {}", localIp);
            return localIp;
        }
        catch (std::exception& ex) {
            LOG_ERROR_D("Caught exception = {}", ex.what());
        }
        return "0.0.0.0";
    }
}
#endif