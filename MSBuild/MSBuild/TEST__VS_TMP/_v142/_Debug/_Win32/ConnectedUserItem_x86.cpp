#include "ConnectedUserItem.h"
#include "HelpersQt.h"

ConnectedUserItem::ConnectedUserItem(uint32_t id, QWidget* parent) : QWidget(parent), id(id) {
	ui.setupUi(this);	
	SetupThemeChanging();
	themeOperations.SetCurrentTheme();

	connect(ui.CloseButton, &QPushButton::clicked, this, [this] {
		emit Removing(this); // first - remove itself from ConnectionDataWindow's layout
		emit Closing();  // then - hide ConnectionDataWindow and removing other users
		});

	SetName("user"); // default name

	// Force signal to retranslate UI buttons
	QCoreApplication::postEvent(this, new QEvent(QEvent::LanguageChange));
}

void ConnectedUserItem::SetName(QString name) {
	ui.UserName->setText(name + (" ("+std::to_string(id)+")").c_str());
}

void ConnectedUserItem::Remove() {
	emit Removing(this);
}

uint32_t ConnectedUserItem::GetId() {
	return id;
}

void ConnectedUserItem::SetupThemeChanging() {
	themeOperations.lightThemeOperation = std::make_shared<std::function<void()>>([this] {
		backgroundColor = "rgba(0, 0, 0, 0.15)";
		fontColor = "#000000";
		SetupStyle();
		});

	themeOperations.darkThemeOperation = std::make_shared<std::function<void()>>([this] {
		backgroundColor = "rgba(255, 255, 255, 0.03)";
		fontColor = "#ffffff";
		SetupStyle();
		});

	themeOperations.AplyThemesCallbacks();
}

void ConnectedUserItem::SetupStyle() {
	this->setStyleSheet(QString{ "background: %1" }.arg(backgroundColor));
	ui.UserName->setStyleSheet(ui.UserName->styleSheet().append(QString{ "color: %1;" }.arg(fontColor)));
	ui.Icon->setStyleSheet(ui.Icon->styleSheet().append(QString{ "color: %1;" }.arg(fontColor)));
	
	HelpersQt::SetStyleSheetFor({ui.CloseButton}, "*", QString{ "color: %1;" }.arg(fontColor));
	HelpersQt::SetStyleSheetFor({ui.CloseButton}, "QToolTip", "border: 0px; background-color: rgba(255,255,255, 0.5); color: black; font-size: 18px;");
}


void ConnectedUserItem::changeEvent(QEvent* event) {
	if (event->type() == QEvent::LanguageChange)
		RetranslateUI();
	QWidget::changeEvent(event);
}

void ConnectedUserItem::RetranslateUI() {
	ui.retranslateUi(this);
	ui.CloseButton->setToolTip(tr("Disconnect user"));
}
