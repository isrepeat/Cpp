#include "AppCenter.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QEventLoop>
#include "../../../../Shared/ComAPI/ComAPI.h"
#include "../../../../Shared/Helpers/Time.h"
#include "../../../../Shared/Helpers/RegistryManager.h"



QJsonObject StackFrame::ToJsonObject() {
    return QJsonObject{
            {"fileName", fileName},
            {"className", className},
            {"methodName", methodName},
            {"lineNumber", lineNumber},
    };
}


AppCenter& AppCenter::GetInstance() {
    static AppCenter instance;
    return instance;
}



AppCenter::AppCenter(QObject* parent)
    : manager(new QNetworkAccessManager(parent))
    , appLaunchTimestamp{ H::GetTimeNow(H::TimeFormat::Ymdhms_with_separators).c_str() }
{
    connect(this, &AppCenter::SendInternal, this, [this](QJsonObject payload) {
        QNetworkRequest request(QString{ "https://in.appcenter.ms/logs?Api-Version=1.0.0" });
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("app-secret", appSecret.toUtf8());
        request.setRawHeader("install-id", instId.toUtf8());

        bool successSending = false;
        for (int attempts = 0; attempts < 4; attempts++) {
            QEventLoop loop;
            // no need delete reply manually its lifetime managed by this->manager
            QNetworkReply* reply = manager->post(request, QJsonDocument{ payload }.toJson());
            connect(reply, &QNetworkReply::finished, &loop, [&] {
                loop.quit();
                });
            loop.exec(); // wait reply

            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            qDebug() << "AppCenter Satus Code = " << statusCode;

            if (reply->error() == QNetworkReply::NoError) {
                QString contents = QString::fromUtf8(reply->readAll());
                qDebug() << "AppCenter OK: " << contents;
                successSending = true;
                break;
            }
            else {
                QString err = reply->errorString();
                qDebug() << "AppCenter FAIL: " << err;
            }
        }
        emit ReportSendingStatus(successSending);
        }, Qt::DirectConnection);
}



void AppCenter::SetApplicationData(const QString& appSecret, const QString& instId, const QString& version) {
    this->appSecret = appSecret;
    this->version = version;
    this->instId = instId;
}

void AppCenter::SendCrashReport(const QString& exceptionMessage, QList<StackFrame> stackFrames, QString rawStackTrace, QList<QDir> attachmentDirs) {
    if (appSecret.size() == 0)
        return; // we try send crash report before initialized AppCenter ...

    auto windowsVersion = QString::fromStdWString(ComApi::WindowsVersion());
    auto productName = QString::fromStdString(H::RegistryManager::GetRegValue(HKey::LocalMachine, "HARDWARE\\DESCRIPTION\\System\\BIOS", "SystemProductName"));

    QJsonArray frames;
    for (auto& stackFrame : stackFrames) {
        frames.append(stackFrame.ToJsonObject());
    }

    // Create hash to be more unique message for AppCenter sorting
    QString stacktraceHash = QCryptographicHash::hash(rawStackTrace.toUtf8(), QCryptographicHash::Md5).toHex();
    QString logErrorId = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);

    QJsonObject logs;
    logs["type"] = "managedError";
    logs["timestamp"] = QString{ H::GetTimeNow(H::TimeFormat::Ymdhms_with_separators).c_str() };
    logs["appLaunchTimestamp"] = appLaunchTimestamp;
    logs["processId"] = "1234"; // TODO: get pid
    logs["processName"] = "com.microsoft.appcenter.demo.project";
    logs["id"] = logErrorId;
    logs["fatal"] = true;
    logs["device"] = QJsonObject{
        {"appVersion", version},
        {"appBuild", "1"},
        {"sdkName", "appcenter.custom"},
        {"sdkVersion", "1.0.0"},
        {"model", productName},
        {"osName", "Windows"},
        {"osVersion", windowsVersion},
        {"locale", "en-US"},
    };
    logs["userId"] = instId;
    logs["exception"] = QJsonObject{
        {"type", "Cpp.RuntimeException"},
        {"message", QString("[%1] %2").arg(stacktraceHash).arg(exceptionMessage)},
        {"frames", frames},
        {"stackTrace", rawStackTrace}, // if rawStackTrace not empty 'frames' ignored !!!
    };

    auto logsArray = QJsonArray{
        logs
    };



    for (auto& dir : attachmentDirs) {
        auto listFiles = dir.entryList(QDir::Files);
        auto dirPath = QDir::toNativeSeparators(dir.path());
        auto dirName = dir.dirName();

        // AS DESIGNED: Normally, each log-folder should contain 1 log-file
        int fileCounter = 0;
        for (auto& filename : listFiles) {
            QFile file(dir.path() + "\\" + filename);
            if (file.open(QIODevice::ReadOnly)) {
                QJsonObject attachment;
                attachment["type"] = "errorAttachment";
                attachment["contentType"] = "application/octet-stream";
                attachment["timestamp"] = QString{ H::GetTimeNow(H::TimeFormat::Ymdhms_with_separators).c_str() };
                attachment["filename"] = QString{"[%1] %2"}.arg(dirName).arg(filename);
                attachment["data"] = QString{ file.readAll().toBase64() };
                attachment["errorId"] = logErrorId;
                attachment["id"] = QUuid::createUuid().toString().remove(0, 1).chopped(1);
                attachment["device"] = QJsonObject{
                   {"appVersion", version},
                   {"appBuild", "1"},
                   {"sdkName", "appcenter.custom"},
                   {"sdkVersion", "1.0.0"},
                   {"model", productName},
                   {"osName", "Windows"},
                   {"osVersion", windowsVersion},
                   {"locale", "en-US"},
                };

                logsArray.push_back(attachment);
            }
        }
    }

    QJsonObject payload;
    payload["logs"] = logsArray;

    emit SendInternal(payload);
}