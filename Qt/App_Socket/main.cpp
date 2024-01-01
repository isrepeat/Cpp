#include "AppMainWindow.h"
#include <QApplication>
#include <iostream>
#include <windows.h>

int main(int argc, char **argv) {
    if (H::StdRedirection::ReAllocConsole()) {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    
    auto window = new AppMainWindow();
    window->resize(800,600);
    window->show();

    return app.exec();
}