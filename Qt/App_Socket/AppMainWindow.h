#pragma once
#include <Helpers/StdRedirection.h>
#include <Helpers/Singleton.hpp>
#include "ui_AppMainWindow.h"

#include <QMainWindow>
#include <QTimer>

#include <condition_variable>
#include <thread>

class AppMainWindow : public QMainWindow {
	Q_OBJECT;

public:
	AppMainWindow(QWidget *parent = nullptr);
	~AppMainWindow();

signals:
	void StdOutput(const QByteArray& outputBuffer);

private slots:
	void GotStdOutput(const QByteArray& outputBuffer);
	void StartConnection();
	void Stop();

private:
	Ui::AppMainWindowClass ui;
	std::condition_variable cvSending;
	std::thread senderThread;
};

using StdRedirectionSingleton = Singleton<H::StdRedirection>;