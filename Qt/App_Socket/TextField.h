#pragma once
#include "ui_TextField.h"
#include <QWidget>

class TextField : public QWidget {
	Q_OBJECT;

public:
	TextField(QWidget *parent = nullptr);
	~TextField();

private:
	Ui::TextFieldClass ui;
};