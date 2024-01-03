#pragma once
#include <Helpers/StdRedirection.h>
#include <Helpers/Singleton.hpp>
#include "ui_AppMainWindow.h"

#include <QMainWindow>
#include <QTimer>

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
};

using StdRedirectionSingleton = Singleton<H::StdRedirection>;