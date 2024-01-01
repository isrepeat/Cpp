#include "AppMainWindow.h"

#include <Helpers/Logger.h>
#include <QDebug>

AppMainWindow::AppMainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	stdRedirection.BeginRedirect([this](std::vector<char> outputBuffer) {
		ui.consoleWindow->putData(QByteArray(outputBuffer.data(), outputBuffer.size()));
		});

	lg::DefaultLoggers::Init("D:\\App_Socket.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);

	printf("[printf] AAA \n");
	LOG_DEBUG_D("[Log] Redirected AAAAAAAA BBBBBBBBB CCCCCCCCC DDDDDDDDD EEEEEEEEE FFFFFFFFFF GGGGGGGGG ");
	printf("[printf] BBB \n");

}

AppMainWindow::~AppMainWindow() {
}