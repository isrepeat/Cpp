
#include "ColorButton.h"
#include <QDebug>

ColorButton::ColorButton(const QColor& color, const QColor& selectionColor, QWidget* parent) 
	: SelectableButton(SelectableButtonType::Unknown, parent)
	, color{ color }
	, selectionColor{selectionColor}
{
	ui.setupUi(this);
	ui.Mark->setVisible(false);

	setAttribute(Qt::WA_StyledBackground, true);

	auto style = unselectedStyle.arg(color.name());
	setStyleSheet(style);

	//accentColorChangeHandler = std::make_shared<std::function<void(const QColor&)>>([this](const QColor& newColor) {
	//	this->color = newColor;
	//	if (selected)
	//		SetSelected();
	//	else
	//		SetUnselected(); });
}

void ColorButton::SetSelected() {
	selected = true;
	setStyleSheet(selectedStyle.arg(color.name(), selectionColor.name()));
	ui.Mark->setVisible(true);
}

void ColorButton::SetUnselected() {
	selected = false;
	setStyleSheet(unselectedStyle.arg(color.name()));
	ui.Mark->setVisible(false);
}

QColor ColorButton::GetColor() {
	return color;
}
