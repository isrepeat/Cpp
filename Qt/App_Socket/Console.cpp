#include "Console.h"
#include <QScrollBar>

Console::Console(QWidget* parent)
    : QPlainTextEdit(parent)
{
    document()->setMaximumBlockCount(100);
    QPalette p = palette();
    p.setColor(QPalette::Base, QColor{ 0, 0, 0 });
    p.setColor(QPalette::Text, QColor{ 204, 204, 204 });
    setPalette(p);

    setFont(QFont{ "Consolas" });
}

void Console::putData(const QByteArray& data) {
    insertPlainText(data);

    QScrollBar* bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void Console::setLocalEchoEnabled(bool set) {
    m_localEchoEnabled = set;
}

void Console::keyPressEvent(QKeyEvent* e) {
    switch (e->key()) {
    case Qt::Key_Backspace:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        break;
    default:
        if (m_localEchoEnabled) {
            QPlainTextEdit::keyPressEvent(e);
        }
        emit getData(e->text().toLocal8Bit());
    }
}

void Console::mousePressEvent(QMouseEvent* e) {
    Q_UNUSED(e);
    setFocus();
}

void Console::mouseDoubleClickEvent(QMouseEvent* e) {
    Q_UNUSED(e);
}

void Console::contextMenuEvent(QContextMenuEvent* e) {
    Q_UNUSED(e);
}