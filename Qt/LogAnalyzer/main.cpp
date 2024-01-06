// Use predefined macro WIN32_LEAN_AND_MEAN to solve errors like this: "C2011: 'sockaddr': 'struct' type redefinition"
#include <Helpers/Logger.h>
#include "AppMainWindow.h"
#include <QApplication>
#include <windows.h>
#include <iostream>

int main(int argc, char **argv) {
    lg::DefaultLoggers::Init("D:\\LogAnalyzer.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    auto appMainWindow = new AppMainWindow();
    appMainWindow->resize(800,600);
    appMainWindow->show();

    return app.exec();
}