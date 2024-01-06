#include "AppMainWindow.h"
#include <Helpers/Logger.h>
#include <Helpers/Time.h>
#include "Common.h"
#include <QDebug>

AppMainWindow::AppMainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	connect(this, &AppMainWindow::StdOutput, this, &AppMainWindow::GotStdOutput);

	StdRedirectionSingleton::GetInstance().SetCallback([this](std::vector<char> outputBuffer) {
		emit StdOutput(QByteArray(outputBuffer.data(), outputBuffer.size()));
		});
	printf("\n"); // force print accumulated data from buffer

	connect(ui.btnStartConnection, &QPushButton::clicked, this, &AppMainWindow::StartConnection);
	connect(ui.btnStop, &QPushButton::clicked, this, &AppMainWindow::Stop);
}

AppMainWindow::~AppMainWindow() {
	LOG_FUNCTION_ENTER("~AppMainWindow()");
	Stop();
	LOG_DEBUG("Exit");
}

void AppMainWindow::GotStdOutput(const QByteArray& outputBuffer) {
	ui.consoleWindow->putData(outputBuffer);
}

void AppMainWindow::StartConnection() {
	LOG_FUNCTION_ENTER("StartConnection()");

	senderThread = std::thread([this] {
		boost::asio::io_service io_service;
		udp::socket socket(io_service);

		LOG_DEBUG("Open socket");
		udp::endpoint remote_endpoint = udp::endpoint(address::from_string(IPADDRESS), UDP_PORT);
		socket.open(udp::v4());

		LOG_DEBUG("Send to {}", remote_endpoint.address().to_string());
		boost::system::error_code err;
		auto sent = socket.send_to(boost::asio::buffer("test message"), remote_endpoint, 0, err);

		std::mutex mx;
		std::unique_lock lk{ mx };
		cvSending.wait_for(lk, 20'000ms);

		LOG_DEBUG("Close socket");
		socket.close();
		});
}

void AppMainWindow::Stop() {
	LOG_FUNCTION_SCOPE("Stop()");
	
	cvSending.notify_one();
	if (senderThread.joinable())
		senderThread.join();
}