#pragma once
#include <QPushButton>
#include "ui_ColorButton.h"
#include "SelectableButton.h"

//TODO add theme changing
class ColorButton : public SelectableButton
{
	Q_OBJECT

public:
	ColorButton(const QColor& color, const QColor& selectionColor, QWidget* parent = Q_NULLPTR);
	~ColorButton() = default;

	void SetSelected() final;
	void SetUnselected() final;
	QColor GetColor();

private:
	Ui::ColorButton ui;

	QString unselectedStyle = "background-color:%1;border: 0px;";
	QString selectedStyle = "background-color:%1;border: 1px solid %2;";

	QColor color;
	QColor selectionColor;
	bool selected = false;
	/*std::shared_ptr<std::function<void(const QColor&)>> accentColorChangeHandler;*/
};