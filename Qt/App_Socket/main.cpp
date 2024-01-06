// Use predefined macro WIN32_LEAN_AND_MEAN to solve errors like this: "C2011: 'sockaddr': 'struct' type redefinition"
#include "AppMainWindow.h"
#include <QApplication>
#include <iostream>
#include <windows.h>

int main(int argc, char **argv) {
    StdRedirectionSingleton::CreateInstance();
    if (StdRedirectionSingleton::GetInstance().ReAllocConsole()) {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }
    StdRedirectionSingleton::GetInstance().BeginRedirect(nullptr);

    lg::DefaultLoggers::Init("D:\\App_Socket.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    
    auto window = new AppMainWindow();
    window->resize(800,600);
    window->show();

    return app.exec();
}