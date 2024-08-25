#pragma once

#include <QWidget>
#include "ui_ButtonsPanel.h"
#include "SelectableButton.h"

class ButtonsPanel : public QWidget {
	Q_OBJECT;

public:
	ButtonsPanel(QWidget *parent = Q_NULLPTR);
	~ButtonsPanel() = default;

	void AddButton(SelectableButton* button);
	SelectableButton* GetButton(SelectableButtonType type);

	void SelectButton(int index);
	void ClickButton(int index);
	void Clear();

private slots:
	void ChildButtonClicked();

private:
	Ui::ButtonsPanel ui;
	std::vector<SelectableButton*> buttons;
};