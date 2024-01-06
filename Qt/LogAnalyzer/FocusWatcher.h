#pragma once
#include <QObject>
#include <QEvent>

class FocusWatcher : public QObject {
	Q_OBJECT;

public:
	explicit FocusWatcher(QObject* parent = nullptr);
	bool eventFilter(QObject* obj, QEvent* event) override;

signals:
	void FocusIn();
	void FocusOut();
};