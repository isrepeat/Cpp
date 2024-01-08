#pragma once
#include "ui_AppMainWindow.h"
#include <QMainWindow>

class AppMainWindow : public QMainWindow {
	Q_OBJECT;

public:
	AppMainWindow(QWidget *parent = nullptr);
	~AppMainWindow();

signals:

private slots:
	void OpenDocument();
	void CloseDocument();

private:
	Ui::AppMainWindowClass ui;
};