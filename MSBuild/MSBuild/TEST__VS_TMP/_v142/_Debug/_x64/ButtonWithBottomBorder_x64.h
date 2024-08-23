#pragma once

#include <QPushButton>
#include "ui_ButtonWithBottomBorder.h"
#include "SelectableButton.h"
#include "ThemeChangeHelper.h"

class ButtonWithBottomBorder : public SelectableButton {
	Q_OBJECT;

public:
	ButtonWithBottomBorder(
		QString icon,
		std::string text,
		uint64_t startSpacerWidth = 20,
		uint64_t betweenElementSpacerWidth = 10,
		uint64_t endSpacerWidth = 5,
		SelectableButtonType buttonType = SelectableButtonType::Unknown,
		QWidget *parent = Q_NULLPTR
	);
	~ButtonWithBottomBorder() = default;

	void SetSelected() override;
	void SetUnselected() override;

	void SetBorderColor(QColor color);
	void SetIconSize(QSize newSize);

private:
	void SetupThemeChanging();

	void changeEvent(QEvent* event) final;
	void RetranslateUI();
	
	void SetFixedSize();

private:
	Ui::ButtonWithBottomBorder ui;
	QColor borderColor;

	QString selectedTextStyle = "*{"
						"color: %2;"
						"font-weight: 450;"
						"font-size: 18px;"
						"font-style: normal;"
						"line-height: normal;"
						"font-family: %1;"
						"border: 0px;"
						"}";

	QString unselectedTextStyle = "*{"
		"color: %2;"
		"font-weight: 450;"
		"font-size: 18px;"
		"font-style: normal;"
		"line-height: normal;"
		"font-family: %1;"
		"}";

	QString selectedButtonStyle = "*{"
		"border: 0px;"
		"border-bottom: 2px solid %1;"
		"}";

	QString unselectedButtonStyle = "*{"
		"border: 0px;"
		"}";

	QString hoverButtonStyle = ""
		"background-color: rgba(150,150,150, 0.2);"
		"";


	QString selectedFontColor;
	QString unselectedFontColor;

	bool selected = false;

	ThemeChangeHelper themeOperations;
	std::shared_ptr<std::function<void(const QColor&)>> accentColorChangeHandler;

	std::string text;
};
