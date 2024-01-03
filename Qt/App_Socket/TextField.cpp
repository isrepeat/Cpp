#include "TextField.h"

TextField::TextField(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

TextField::~TextField() {
}

void TextField::SetName(QString name) {
	ui.label->setText(name);
}

void TextField::SetText(QString text) {
	ui.lineEdit->setText(text);
}

QString TextField::GetText() {
	return ui.lineEdit->text();
}
