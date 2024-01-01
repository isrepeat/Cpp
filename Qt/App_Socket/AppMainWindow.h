#pragma once
#include <Helpers/StdRedirection.h>
#include "ui_AppMainWindow.h"

#include <QMainWindow>
#include <QTimer>

class AppMainWindow : public QMainWindow {
	Q_OBJECT;

public:
	AppMainWindow(QWidget *parent = nullptr);
	~AppMainWindow();

public slots:

private:
	Ui::AppMainWindowClass ui;
	H::StdRedirection stdRedirection;
};
