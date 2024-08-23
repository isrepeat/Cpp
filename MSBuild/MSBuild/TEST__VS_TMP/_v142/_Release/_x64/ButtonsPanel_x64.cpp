#include "ButtonsPanel.h"

ButtonsPanel::ButtonsPanel(QWidget *parent) 
	: QWidget(parent) 
{
	ui.setupUi(this);
}


void ButtonsPanel::AddButton(SelectableButton* button) {
	buttons.push_back(button);
	layout()->addWidget(button);
	connect(button, &QPushButton::clicked, this, &ButtonsPanel::ChildButtonClicked);
}

// TODO: return array of buttons that matching 'type'
SelectableButton* ButtonsPanel::GetButton(SelectableButtonType type) {
	for (auto button : buttons) {
		if (button->GetButtonType() == type) {
			return button;
		}
	}
	return nullptr;
}

void ButtonsPanel::SelectButton(int index) {
	if (index >= buttons.size())
		return;

	for (int i = 0; i < buttons.size(); i++) {
		if (i == index) {
			buttons[i]->SetSelected();
		}
		else {
			buttons[i]->SetUnselected();
		}
	}
}

void ButtonsPanel::ClickButton(int index) {
	if (index >= buttons.size())
		return;

	buttons[index]->click();
}

void ButtonsPanel::Clear() {
	while (QLayoutItem* item = layout()->takeAt(0)) {
		delete item->widget();
		delete item;
	}
	buttons.clear();
}

void ButtonsPanel::ChildButtonClicked() {
	auto sender = QObject::sender();

	for (auto& button : buttons)
		if (button != sender)
			button->SetUnselected();
		else
			button->SetSelected();
}