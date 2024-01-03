#include "MainWindow.h"
#include <QApplication>
#include <QMimeData>
#include <QLayout>
#include <QDebug>
#include <QTimer>
#include <thread>

#include <Windows.h>
#include <QProcess>


MainWindow::MainWindow(QWidget* parent) 
	: QMainWindow(parent) 
{
	QWidget* centralWidget = new QWidget(this);
	centralWidget->setStyleSheet("QWidget{border: 1px solid green;}");

	QVBoxLayout* vLayout = new QVBoxLayout(centralWidget);
	centralWidget->setLayout(vLayout);
	setCentralWidget(centralWidget);

	QWidget* innerWidget = new QWidget(this);
	innerWidget->setStyleSheet("QWidget{border: 2px solid red;}");
	innerWidget->setFixedHeight(200);
	vLayout->addWidget(innerWidget);

	auto vvLayout = new QVBoxLayout(innerWidget);
	auto hhLayout = new QHBoxLayout();
	auto hhLayout2 = new QHBoxLayout();


	auto lbA = new QLabel("Some text label");
	lbA->setStyleSheet("QLabel{color: blue; border: 2px solid blue;}");
	//hhLayout->setAlignment(Qt::AlignBottom);
	hhLayout->addWidget(lbA);


	auto pbA = new QPushButton("Some button text");
	pbA->setStyleSheet("QPushButton{color: brown; border: 2px solid brown;}");
	//hhLayout2->setAlignment(Qt::AlignBottom);
	hhLayout2->addWidget(pbA);


	//vvLayout->addSpacing(100);
	vvLayout->addLayout(hhLayout);
	vvLayout->addLayout(hhLayout2);


	connect(pbA, &QPushButton::clicked, [this] {
		//Beep(500, 500);
		//Sleep(1500);
		QString program = qApp->arguments()[0];
		QStringList arguments = qApp->arguments().mid(1); // remove the 1st argument - the program name
		QProcess::startDetached(program, arguments);
		//qApp->exit(0);
		exit(0);
		Sleep(10000);
		int xxx = 9;
		});
}