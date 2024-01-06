#pragma once
#include "ui_TextField.h"
#include <QWidget>

class TextField : public QWidget {
	Q_OBJECT;

public:
	TextField(QWidget* parent = nullptr);
	~TextField();

	void SetName(QString name);
	void SetText(QString text);
	QString GetText();

private:
	Ui::TextFieldClass ui;
};