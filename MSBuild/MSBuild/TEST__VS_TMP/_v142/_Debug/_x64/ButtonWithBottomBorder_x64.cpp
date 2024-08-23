#include "ButtonWithBottomBorder.h"
#include <QSpacerItem>
#include "ThemeObserver.h"
#include "Translator.h"
#include "HelpersQt.h"

//TODO take saved color from app settings
ButtonWithBottomBorder::ButtonWithBottomBorder(
	QString icon,
	std::string text,
	uint64_t startSpacerWidth,
	uint64_t betweenElementSpacerWidth,
	uint64_t endSpacerWidth,
	SelectableButtonType buttonType,
	QWidget* parent
)
	: SelectableButton(buttonType, parent)
	, borderColor{ AccentColor(AccentColor::Purple).GetColor() }
	, text{text}
{
	ui.setupUi(this);

	this->SetUnselected();

	ui.Icon->setAttribute(Qt::WA_TranslucentBackground, true);
	ui.Text->setAttribute(Qt::WA_TranslucentBackground, true);

	ui.Icon->setText(icon);
	ui.Text->setText(Translator::tr(text.c_str()));
	ui.Text->adjustSize();

	ui.StartSpacer->changeSize(startSpacerWidth, ui.StartSpacer->geometry().height());
	ui.BetweenElementSpacer->changeSize(betweenElementSpacerWidth, ui.BetweenElementSpacer->geometry().height());
	ui.EndSpacer->changeSize(endSpacerWidth, ui.EndSpacer->geometry().height());
	
	accentColorChangeHandler = std::make_shared<std::function<void(const QColor&)>>([this](const QColor& newColor) {
		this->borderColor = newColor;
		if (selected) {
			this->SetSelected();
		}
		else {
			this->SetUnselected();
		}
		});

	ThemeObserver::GetInstance().AddNewAccentColorOperation(accentColorChangeHandler);

	this->SetupThemeChanging();
	this->SetFixedSize();
}

void ButtonWithBottomBorder::SetSelected() {
	selected = true;
	this->setStyleSheet(selectedButtonStyle.arg(borderColor.name()) + " QPushButton:hover{"+hoverButtonStyle+"}");


	ui.Icon->setStyleSheet(QString{ selectedTextStyle }.arg("\"Segoe MDL2 Assets\"", selectedFontColor));
	ui.Text->setStyleSheet(QString{ selectedTextStyle }.arg("\"Segoe UI\"", selectedFontColor));	
}

void ButtonWithBottomBorder::SetUnselected() {
	selected = false;
	this->setStyleSheet(unselectedButtonStyle + " QPushButton:hover{"+hoverButtonStyle+"}");

	ui.Icon->setStyleSheet(QString{ unselectedTextStyle }.arg("\"Segoe MDL2 Assets\"", unselectedFontColor));
	ui.Text->setStyleSheet(QString{ unselectedTextStyle }.arg("\"Segoe UI\"", unselectedFontColor));
}

void ButtonWithBottomBorder::SetBorderColor(QColor color) {
	borderColor = color;
}

void ButtonWithBottomBorder::SetIconSize(QSize newSize) {
	ui.Icon->setFixedSize(newSize);
	this->SetFixedSize();
}

void ButtonWithBottomBorder::SetupThemeChanging() {
	themeOperations.darkThemeOperation = std::make_shared<std::function<void()>>([this] {
		selectedFontColor = "rgb(255,255,255)";
		unselectedFontColor = "rgba(255,255,255,0.6)";
		hoverButtonStyle = HelpersQt::SetStyleSheetProperty(hoverButtonStyle, "background-color: rgba(90,90,90, 0.2);");
		if (selected) {
			this->SetSelected();
		}
		else {
			this->SetUnselected();
		}
		});	
	
	themeOperations.lightThemeOperation = std::make_shared<std::function<void()>>([this] {
		selectedFontColor = "rgb(0,0,0)";
		unselectedFontColor = "rgba(0,0,0,0.6)";
		hoverButtonStyle = HelpersQt::SetStyleSheetProperty(hoverButtonStyle, "background-color: rgba(150,150,150, 0.2);");
		if (selected) {
			this->SetSelected();
		}
		else {
			this->SetUnselected();
		}
		});

	themeOperations.AplyThemesCallbacks();

}

void ButtonWithBottomBorder::changeEvent(QEvent* event) {
	if (event->type() == QEvent::LanguageChange) {
		this->RetranslateUI();
	}
	QPushButton::changeEvent(event);
}

void ButtonWithBottomBorder::RetranslateUI() {
	ui.Text->setText(Translator::tr(text.c_str()));
	this->SetFixedSize();
}

void ButtonWithBottomBorder::SetFixedSize() {
	//ui.Text->setFixedWidth(ui.Text->sizeHint().width());
	ui.Text->setFixedWidth(ui.Text->fontMetrics().boundingRect(ui.Text->text()).width() + 5);

	auto width = ui.BetweenElementSpacer->sizeHint().width() + ui.Icon->size().width() + ui.StartSpacer->sizeHint().width()
		+ ui.Text->size().width() + ui.EndSpacer->sizeHint().width();

	this->setFixedWidth(width);
}
