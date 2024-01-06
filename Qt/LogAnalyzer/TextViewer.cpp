#include "TextViewer.h"
#include <QScrollBar>

TextViewer::TextViewer(QWidget* parent)
    : QPlainTextEdit(parent)
{
    QPalette p = palette();
    p.setColor(QPalette::Base, QColor{ 0, 0, 0 });
    p.setColor(QPalette::Text, QColor{ 204, 204, 204 });
    this->setPalette(p);
    this->setFont(QFont{ "Consolas" });

    this->document()->setMaximumBlockCount(10'000);
}

void TextViewer::putData(const QByteArray& data) {
    this->moveCursor(QTextCursor::MoveOperation::End);

    this->insertPlainText(data);

    QScrollBar* bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void TextViewer::setKeyEventsEnabled(bool enabled) {
    keyEventsEnabled = enabled;
}

void TextViewer::setMouseEventsEnabled(bool enabled) {
    mauseEventsEnabled = enabled;
}

void TextViewer::keyPressEvent(QKeyEvent* e) {
    switch (e->key()) {
        //case Qt::Key_Backspace:
        //case Qt::Key_Left:
        //case Qt::Key_Right:
        //case Qt::Key_Up:
        //case Qt::Key_Down:
        //    break;
    default:
        if (keyEventsEnabled) {
            QPlainTextEdit::keyPressEvent(e);
        }
        emit getData(e->text().toLocal8Bit());
    }
}

//// Ignore some mouse events:
//void TextViewer::mousePressEvent(QMouseEvent* e) {
//    if (mauseEventsEnabled) {
//        QPlainTextEdit::mousePressEvent(e);
//    }
//    else {
//        Q_UNUSED(e);
//        setFocus();
//    }
//}
//
//void TextViewer::mouseDoubleClickEvent(QMouseEvent* e) {
//    Q_UNUSED(e);
//}
//void TextViewer::contextMenuEvent(QContextMenuEvent* e) {
//    Q_UNUSED(e);
//}