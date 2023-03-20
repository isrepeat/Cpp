#pragma once
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QObject>
#include <QList>
#include <QDir>
#include <QUrl>

// TODO: add namespace to avoid conflicts with StacktraceRestorer nuget
struct StackFrame {
    QString fileName;
    QString className;
    QString methodName;
    int lineNumber;

    QJsonObject ToJsonObject();
};


class AppCenter : public QObject
{
    Q_OBJECT // uncomment if use inheritance

    AppCenter(QObject* parent = nullptr);
public:
    ~AppCenter() = default;

    static AppCenter& GetInstance();

    void SetApplicationData(const QString& appSecret, const QString& instId, const QString& version);
    void SendCrashReport(const QString& exceptionMessage, QList<StackFrame> stackFrames, QString rawStackTrace = "", QList<QDir> attachmentDirs = {});

    // TODO: use private signals
signals:
    void Send(QJsonObject payload);

private:
    QString appSecret;
    QString instId = "00000000-0000-0000-0000-000000000001"; // use default id before we set it up manually
    QString version;

    QString appLaunchTimestamp;

    QNetworkAccessManager* manager;
    QUrl url;
};