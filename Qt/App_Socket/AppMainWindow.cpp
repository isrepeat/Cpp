#include "AppMainWindow.h"
#include <Helpers/Logger.h>
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
}

void AppMainWindow::Stop() {
	LOG_FUNCTION_SCOPE("Stop()");
}