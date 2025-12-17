#pragma once
#include "common.h"
#include "HWindows.h"
#include "Logger.h"

#include <vector>
#if COMPILE_FOR_DESKTOP || COMPILE_FOR_CX
#include <fstream>
#include <atomic>
#include <span>
#endif

#if COMPILE_FOR_WINRT
#include <ppltasks.h>	// For create_task
#endif


namespace HELPERS_NS {
#if COMPILE_FOR_DESKTOP || COMPILE_FOR_CX
#define READ_FILE_BUFFER_SIZE_DEFAULT 1024
#define READ_FILE_BUFFER_SIZE_MAX (10*1024*1024) // 10 mb
    
    inline BOOL CloseHandleSafe(HANDLE h) {
        return h && h != INVALID_HANDLE_VALUE ? CloseHandle(h) : TRUE;
    }

    enum class StatusReadFile {
        Error,
        Stopped,
        Completed,
        NeedMoreBuffer,
    };

    enum class StatusWriteFile {
        Error,
        Stopped,
        Completed,
    };

    // TODO: wrap to 'details' ns
    inline StatusReadFile GetOverlappedResultRoutine(HANDLE hFile, LPOVERLAPPED lpOverlapped, LPDWORD numberOfBytesTransfered, const std::atomic<bool>& stop) {
        StatusReadFile status;

        while (true) {
            // Check break conditions in while scope (if we chek it after while may be status conflicts)
            if (stop) {
                return StatusReadFile::Stopped;
            }

            if (::GetOverlappedResult(hFile, lpOverlapped, numberOfBytesTransfered, FALSE)) {
                return StatusReadFile::Completed;
            }
            else { // pending ...
                auto lastErrorGetOverlappedResult = ::GetLastError();
                switch (lastErrorGetOverlappedResult) {
                case ERROR_IO_INCOMPLETE:
                    break; // continue call GetOverlappedResult() until it finish or error

                case ERROR_MORE_DATA:
                    return StatusReadFile::NeedMoreBuffer;

                default:
                    LogLastError;
                    return StatusReadFile::Error;
                }
            }
        }
    }


    // TODO: rewrite this without blocking current thread
    template<typename T = uint8_t>
    StatusReadFile ReadFileAsync(HANDLE hFile, const std::atomic<bool>& stop, std::vector<T>& outBuffer, DWORD numberOfBytesToRead) {
        StatusReadFile status;

        DWORD dwBytesRead = 0;
        OVERLAPPED stOverlapped = { 0 };
        std::size_t outBufferOldSize = 0;

        do {
            outBufferOldSize = outBuffer.size();
            outBuffer.resize(outBufferOldSize + numberOfBytesToRead);

            // Read new data after outBuffer.data() + outBufferOldSize to save old data
            if (::ReadFile(hFile, outBuffer.data() + outBufferOldSize, numberOfBytesToRead * sizeof(T), &dwBytesRead, &stOverlapped)) {
                // ReadFile completed synchronously ...
                // NOTE: if dwBytesRead == 0 it is mean that other side call WriteFile with zero data
                status = StatusReadFile::Completed;
            }
            else {
                auto lastErrorReadFile = GetLastError();
                switch (lastErrorReadFile) {
                case ERROR_IO_PENDING:
                    status = GetOverlappedResultRoutine(hFile, &stOverlapped, &dwBytesRead, stop);
                    break;

                case ERROR_MORE_DATA:
                    status = StatusReadFile::NeedMoreBuffer;
                    break;

                default:
                    LogLastError;
                    status = StatusReadFile::Error;
                }

                if (status == StatusReadFile::NeedMoreBuffer) {
                    numberOfBytesToRead += READ_FILE_BUFFER_SIZE_DEFAULT * 100;
                    if (numberOfBytesToRead >= READ_FILE_BUFFER_SIZE_MAX) {
                        status = StatusReadFile::Error;
                        LogLastError;
                        break;
                    }
                }
            }
        } while (status == StatusReadFile::NeedMoreBuffer);

        switch (status) {
        case StatusReadFile::Error:
            throw std::exception("ReadFile error");
            break;

        case StatusReadFile::Stopped:
            throw std::exception("Was stopped signal");
            break;

        case StatusReadFile::Completed:
            if (outBufferOldSize + dwBytesRead == 0) {
                throw std::exception("Zero data was read: 'outBufferOldSize + dwBytesRead == 0'");
            }
        }

        // TODO: Check if "(dwBytesRead) / sizeof(T)" is correct expression on uint16_t+
        outBuffer.resize(outBufferOldSize + (dwBytesRead) / sizeof(T)); // truncate
        return status;
    }


    template<typename T = uint8_t>
    StatusWriteFile WriteFileAsync(HANDLE hFile, const std::atomic<bool>& stop, std::span<T> writeData) {
        StatusWriteFile status;

        DWORD cbWritten = 0;
        OVERLAPPED stOverlapped = { 0 };

        if (::WriteFile(hFile, writeData.data(), writeData.size() * sizeof(T), &cbWritten, &stOverlapped)) {
            // WriteFile completed synchronously ...
            status = StatusWriteFile::Completed;
        }
        else {
            auto lastErrorReadFile = ::GetLastError();
            switch (lastErrorReadFile) {
            case ERROR_IO_PENDING:
                while (true) {
                    // Check break conditions in while scope (if we chek it after while may be status conflicts)
                    if (stop) {
                        status = StatusWriteFile::Stopped;
                        break;
                    }

                    if (::GetOverlappedResult(hFile, &stOverlapped, &cbWritten, FALSE)) {
                        // ReadFile operation completed
                        status = StatusWriteFile::Completed;
                        break;
                    }
                    else {
                        // pending ...
                        auto lastErrorGetOverlappedResult = ::GetLastError();
                        if (lastErrorGetOverlappedResult != ERROR_IO_INCOMPLETE) {
                            // remote side was closed or smth else
                            status = StatusWriteFile::Error;
                            LogLastError;
                            break;
                        }
                    }
                }
                break;

            default:
                LogLastError;
                status = StatusWriteFile::Error;
            }
        }

        switch (status) {
        case StatusWriteFile::Error:
            throw std::exception("WriteFile error");
            break;

        case StatusWriteFile::Stopped:
            throw std::exception("Was stopped signal");
            break;

        case StatusWriteFile::Completed: // mb unreachable
            if (cbWritten == 0) {
                throw std::exception("Zero bytes written: 'cbWritten == 0'");
            }
        }

        return status;
    }
#endif

#if COMPILE_FOR_WINRT
    namespace WinRt {
        // Function that reads from a binary file asynchronously.
        inline Concurrency::task<std::vector<uint8_t>> ReadDataAsync(const std::filesystem::path& filepath) {
            auto installedFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;
            std::filesystem::path installedLocation = installedFolder->Path->Data();

            LOG_ASSERT(std::filesystem::exists(installedLocation / filepath), "File not exist");

            auto filepathParentDirs = filepath;
            filepathParentDirs._Remove_filename_and_separator();

            auto taskGetFolder = Concurrency::task_from_result(installedFolder);

            for (auto& pathItem : filepathParentDirs) {
                taskGetFolder = taskGetFolder.then([pathItem](Windows::Storage::StorageFolder^ folder) {
                    std::filesystem::path folderPath = folder->Path->Data();
                    return Concurrency::create_task(folder->GetFolderAsync(Platform::StringReference(pathItem.wstring().c_str())));
                    });
            }

            return taskGetFolder.then([filepath](Windows::Storage::StorageFolder^ folder) {
                std::filesystem::path folderPath = folder->Path->Data();
                return Concurrency::create_task(folder->GetFileAsync(Platform::StringReference(filepath.filename().wstring().c_str())));
                })
                .then([](Windows::Storage::StorageFile^ file) {
                    return Windows::Storage::FileIO::ReadBufferAsync(file);
                    })
                    .then([](Windows::Storage::Streams::IBuffer^ fileBuffer) -> std::vector<uint8_t> {
                        std::vector<uint8_t> returnBuffer;
                        returnBuffer.resize(fileBuffer->Length);
                        Windows::Storage::Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(Platform::ArrayReference<BYTE>(returnBuffer.data(), fileBuffer->Length));
                        return returnBuffer;
                        });
        }
    }
#endif
}