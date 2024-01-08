#pragma once
#include <QPlainTextEdit>

class TextViewer : public QPlainTextEdit {
    Q_OBJECT;
public:
    explicit TextViewer(QWidget* parent = nullptr);

    void putData(const QByteArray& data);
    void setKeyEventsEnabled(bool enabled);
    void setMouseEventsEnabled(bool enabled);

signals:
    void getData(const QByteArray& data);

protected:
    void keyPressEvent(QKeyEvent* e) override;
    //void mousePressEvent(QMouseEvent* e) override;
    //void mouseDoubleClickEvent(QMouseEvent* e) override;
    //void contextMenuEvent(QContextMenuEvent* e) override;

private:
    bool keyEventsEnabled = false;
    bool mauseEventsEnabled = false;
};