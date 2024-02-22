#pragma once
#include <asio.hpp>
#include <amqpcpp.h>
#include "AMQPBuffer.h"

class AMQPHandler : public AMQP::ConnectionHandler {
public:
    AMQPHandler(const std::string& host, uint16_t port);
    virtual ~AMQPHandler();

    void StartLoop();
    void Quit();
    void Close();

    bool IsQuit() const;
    bool IsConnected() const;
    bool IsSocketOpen() const;

private:
    void onConnected(AMQP::Connection* connection);
    void onData(AMQP::Connection* connection, const char* data, size_t size) final;
    void onError(AMQP::Connection* connection, const char* message) final;
    void onClosed(AMQP::Connection* connection) final;

    std::uint16_t onNegotiate(AMQP::Connection* connection, std::uint16_t interval) final;

    void SendDataFromBuffer();

private:
    static constexpr size_t BUFFER_SIZE = 8 * 1024 * 1024; //8Mb
    static constexpr size_t TEMP_BUFFER_SIZE = 1024 * 1024; //1Mb

    asio::io_context context;
    asio::ip::tcp::socket socket;

    AMQPBuffer outBuffer;
    AMQPBuffer inputBuffer;
    AMQP::Connection* connection = nullptr;
    std::vector<char> tmpBuff;

    std::atomic<bool> quit = false;
    std::atomic<bool> connected = false;
};

