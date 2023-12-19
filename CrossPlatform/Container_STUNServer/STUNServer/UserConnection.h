#pragma once
#include "Platform.h"
#include <asio.hpp>
#include <memory>
#include <functional>
#include <iostream>
#include <functional>


using namespace asio::ip;
using namespace std::string_literals;

class UserConnection : public std::enable_shared_from_this<UserConnection> {
public:
    typedef std::shared_ptr<UserConnection> pointer;

    static pointer Create(asio::io_context& io_context) {
        return pointer(new UserConnection(io_context));
    }

    tcp::socket& Socket() {
        return tcpSocket;
    }
    //asio::streambuf& OutgointMessage() {
    //    return outgointMessage;
    //}

    void WriteIpData(){
        std::cout << "Enter WriteIpData()\n";

        auto endpoint = tcpSocket.remote_endpoint();
        auto adress = endpoint.address().to_string();
        auto port = endpoint.port();
        message = adress + ";"s + std::to_string(port) + "|";

        /*asio::async_read_until(tcpSocket, localClientAddress, '\n', std::bind(&UserConnection::HandleReadClientLocalIP, shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));*/
        //asio::async_write(tcpSocket, asio::buffer(message),
        //    std::bind(&UserConnection::handle_write, shared_from_this(),
        //        std::placeholders::_1,
        //        std::placeholders::_2));
    }

    void WritePartnerData(std::string message) {
        responseToClient = message;
        tcpSocket.async_send(asio::buffer(responseToClient),
            std::bind(&UserConnection::handle_write, shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
    }

    void GetAuthData(std::function<void(std::string)> callback) {
       /* if (localClientAddress.size() == 0) {
            tcpSocket.async_wait(asio::socket_base::wait_read, [this, callback](const asio::error_code& error) {
                callback(this->message);
                });
        }

        callback(message);*/
        asio::async_read_until(tcpSocket, localClientAddress, '\n', std::bind(&UserConnection::HandleReadClientLocalIP, shared_from_this(), callback,
            std::placeholders::_1,
            std::placeholders::_2));
    }
private:
    UserConnection(asio::io_context& io_context) : tcpSocket(io_context){
 
        
    }

    void handle_write(const asio::error_code& error, size_t bytes_transferred) {
        int stop = 354;
    }
    void HandleReadClientLocalIP(std::function<void(std::string)> sendCallback,const asio::error_code& error, size_t bytes_transferred) {
        message += asio::buffer_cast<const char*>(localClientAddress.data());
        std::cout << message;
        std::cout.flush();
        sendCallback(message);
    }

    tcp::socket tcpSocket;
    std::string message;
    std::string responseToClient;
    asio::streambuf localClientAddress;
};