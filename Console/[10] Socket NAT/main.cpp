#include <Helpers/Logger.h>
#include "Common.h"
#include "Client.h"
#include <thread>


namespace {
    const std::string socketTestMessage = "hello world";
}

void Sender(std::string in) {
    boost::asio::io_service io_service;
    udp::socket socket(io_service);

    udp::endpoint remote_endpoint = udp::endpoint(address::from_string(IPADDRESS), UDP_PORT);
    socket.open(udp::v4());

    boost::system::error_code err;
    auto sent = socket.send_to(boost::asio::buffer(in), remote_endpoint, 0, err);
    socket.close();

    LOG_DEBUG_D("Sent Payload [{}]", sent);
}



int main(int argc, char* argv[]) {
    //setlocale(LC_ALL, ".UTF8");
    lg::DefaultLoggers::Init("D:\\SocketNat.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);

    Client client;
    std::thread receiveThread([&] { 
        client.Receiver(); 
        });

    for (int i = 0; i < 3; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        Sender("Test message");
    }

    if (receiveThread.joinable())
        receiveThread.join();

    //Sender("Message AAA");
    //std::this_thread::sleep_for(std::chrono::milliseconds(300));
    //Sender("Message BBB");
    //std::this_thread::sleep_for(std::chrono::milliseconds(300));
    //Sender(socketTestMessage);
    //std::this_thread::sleep_for(std::chrono::milliseconds(300));
    //Sender("Message CCC");

   
    Sleep(20'000);
    return 0;
}