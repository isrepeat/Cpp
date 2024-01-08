#include "AppMainWindow.h"
#include <Helpers/Logger.h>
#include <Helpers/Time.h>
#include "Common.h"
#include <QDebug>

AppMainWindow::AppMainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(ui.btnOpen, &QPushButton::clicked, this, &AppMainWindow::OpenDocument);
	connect(ui.btnClose, &QPushButton::clicked, this, &AppMainWindow::CloseDocument);

	// Connect text fields:
	connect(ui.textEditField_1, &TextField::LineEditFocusOut, this, [this] {
		// ...
		});
}

AppMainWindow::~AppMainWindow() {
	LOG_FUNCTION_SCOPE("~AppMainWindow()");
	CloseDocument();
}

void AppMainWindow::OpenDocument() {
	LOG_FUNCTION_ENTER("OpenDocument()");
	ui.textViewWindow->putData("Hello world");
}

void AppMainWindow::CloseDocument() {
	LOG_FUNCTION_ENTER("CloseDocument()");
}