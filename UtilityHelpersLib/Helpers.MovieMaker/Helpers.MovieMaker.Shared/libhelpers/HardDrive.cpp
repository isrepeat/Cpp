#include "pch.h"
#include "HardDrive.h"
#include <filesystem>

namespace H {
    long double HardDrive::GetFreeMemoryInGB(const std::wstring& disk) {
        return static_cast<long double>(GetFreeMemory(disk)) / 1024 / 1024 / 1024;
    }

    unsigned long long HardDrive::GetFreeMemory(const std::wstring& disk) {
        __int64 FreeBytesAvailable = 0;
        __int64 TotalNumberOfBytes = 0;
        __int64 TotalNumberOfFreeBytes = 0;
        GetDiskFreeSpaceEx(disk.c_str(),
            (PULARGE_INTEGER)&FreeBytesAvailable,
            (PULARGE_INTEGER)&TotalNumberOfBytes,
            (PULARGE_INTEGER)&TotalNumberOfFreeBytes
        );
        //auto CountHardDriveMemory = TotalNumberOfBytes / 1024 / 1024 / 1024; [not used]
        //long double FHardDriveMemory = TotalNumberOfFreeBytes / 1024 / 1024;
        //long double FreeHardDriveMemory = FHardDriveMemory / 1024;

        return TotalNumberOfFreeBytes;
    }

    unsigned long long HardDrive::GetFilesize(const std::wstring& file) {
        return std::filesystem::file_size(file);
    }
    std::wstring HardDrive::GetDiskLetterFromPath(const std::wstring& path) {
        size_t n = path.find(L"\\");
        if (n != std::wstring::npos) {
            return path.substr(0, n+1);
        }

        return L"C:\\"; // by default
    }
}