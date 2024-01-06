#include "FocusWatcher.h"

FocusWatcher::FocusWatcher(QObject* parent) 
	: QObject(parent) 
{
	if (parent)
		parent->installEventFilter(this);
}

bool FocusWatcher::eventFilter(QObject* obj, QEvent* event) {
	if (event->type() == QEvent::FocusIn)
		emit FocusIn();

	else if (event->type() == QEvent::FocusOut)
		emit FocusOut();

	return false;
}