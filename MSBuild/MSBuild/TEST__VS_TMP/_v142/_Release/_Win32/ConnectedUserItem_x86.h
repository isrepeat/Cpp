#pragma once

#include <QWidget>
#include "ui_ConnectedUserItem.h"
#include "ThemeChangeHelper.h"

class ConnectedUserItem : public QWidget
{
	Q_OBJECT

public:
	ConnectedUserItem(uint32_t id, QWidget *parent = Q_NULLPTR);
	void SetName(QString name);
	void Remove();
	uint32_t GetId();

signals:
	void Closing();
	void Removing(ConnectedUserItem* user);

private:
	void SetupThemeChanging();
	void SetupStyle();

	void changeEvent(QEvent* event) final;
	void RetranslateUI();

private:
	Ui::ConnectedUserItem ui;
	uint32_t id;
	QString fontColor;
	QString backgroundColor;
	ThemeChangeHelper themeOperations;
};
