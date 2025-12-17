#include "pch.h"
#include "NativeLogger.h"

#include "libhelpers\HSystem.h"
#include "libhelpers\HData.h"
#include <sstream>

using namespace logging;

Logger::NativeLogger::NativeLogger(std::wstring path, raw_ptr<Filesystem::IFilesystem> filesystem) {
    auto localFolder = Unique(filesystem->GetLocalFolder());
    this->folder = Unique(localFolder->CreateFolder(L"log", Filesystem::FolderCreateMode::OpenAlways));
    this->file = Unique(this->folder->CreateFile(path, Filesystem::FileCreateMode::CreateUnique));
    this->stream = Unique(this->file->OpenStream(Filesystem::FileAccessMode::ReadWrite));
}

void logging::Logger::NativeLogger::ReportMessage(std::wstring msg) {
    if (this->isLoggingEnabled) {
        std::wstringstream outputStream;
        SYSTEMTIME timeInfo;
        GetLocalTime(&timeInfo);

        outputStream << timeInfo.wHour << ':' << timeInfo.wMinute << ':' << timeInfo.wSecond << ':' << timeInfo.wMilliseconds << "   ";
        outputStream << msg << std::endl;

        auto outputMsg = outputStream.str();
        auto path = this->file->GetPath();

        concurrency::create_task([=]() {
            auto mtx = std::lock_guard(this->fileMtx);

            this->stream->Seek(Filesystem::SeekOrigin::End, 0);
            this->stream->Write(outputMsg.data(), (uint32_t)outputMsg.size() * sizeof(wchar_t));
#ifdef _DEBUG
            OutputDebugString(outputMsg.c_str());
#endif
        });
    }
}

void logging::Logger::NativeLogger::ReportMessage(std::string msg) {
    auto tmp = H::Data::Convert<std::wstring>(msg);
    this->ReportMessage(tmp);
}

void logging::Logger::NativeLogger::SetIfLoggingIsEnabled(bool flag) {
    this->isLoggingEnabled = flag;
}

bool logging::Logger::NativeLogger::GetIfLoggingIsEnabled() {
    return this->isLoggingEnabled;
}




Logger::Singleton& Logger::Singleton::Instance() {
    static Singleton instance;
    return instance;
}




void logging::Logger::InitializeLogger(std::wstring path, raw_ptr<Filesystem::IFilesystem> filesystem) {
    auto& instance = Singleton::Instance();
    auto lk = std::lock_guard(instance.internalLoggerMtx);

    instance.internalLogger = std::make_unique<logging::Logger::NativeLogger>(path, filesystem);
}

void logging::Logger::ReportMessage(std::wstring msg) {
    auto& instance = Singleton::Instance();
    auto lk = std::lock_guard(instance.internalLoggerMtx);

    if (!instance.internalLogger) {
        return;
    }

    instance.internalLogger->ReportMessage(msg);
}

void logging::Logger::ReportMessage(std::string msg) {
    auto& instance = Singleton::Instance();
    auto lk = std::lock_guard(instance.internalLoggerMtx);

    if (!instance.internalLogger) {
        return;
    }

    instance.internalLogger->ReportMessage(msg);
}

void logging::Logger::SetLoggingEnabled(bool flag) {
    auto& instance = Singleton::Instance();
    auto lk = std::lock_guard(instance.internalLoggerMtx);

    if (!instance.internalLogger) {
        return;
    }

    instance.internalLogger->SetIfLoggingIsEnabled(flag);
}

bool logging::Logger::GetIsLoggingEnabled() {
    auto& instance = Singleton::Instance();
    auto lk = std::lock_guard(instance.internalLoggerMtx);

    if (!instance.internalLogger) {
        return false;
    }

    return instance.internalLogger->GetIfLoggingIsEnabled();
}
