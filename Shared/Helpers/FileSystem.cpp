#include "FileSystem.h"
#include <filesystem>
#include <algorithm>
#include <fstream>

namespace H {
    bool FileSystem::RemoveFile(const std::wstring& filename) {
        return std::filesystem::remove(filename);
    }

    void FileSystem::RenameFile(const std::wstring& oldFilename, const std::wstring& newFilename) {
        return std::filesystem::rename(oldFilename, newFilename);
    }

    std::vector<uint8_t> FileSystem::ReadFile(const std::wstring& filename) {
        // TODO: add auto find filesize
        std::vector<uint8_t> buff(512, '\0');
        std::ifstream inFile;
        inFile.open(filename, std::ios::binary);
        inFile.read((char*)buff.data(), 512);
        inFile.close();

        return std::move(buff);
    }

    void FileSystem::WriteFile(const std::wstring& filename, const std::vector<uint8_t>& fileData) {
        std::ofstream outFile;
        outFile.open(filename, std::ios::binary);
        outFile.write((char*)fileData.data(), fileData.size());
        outFile.close();
    }


    FileHeader FileSystem::ReadFileHeader(std::wstring filename) {
        std::replace(filename.begin(), filename.end(), L'/', L'\\');

        HANDLE hFile = ::CreateFileW(filename.c_str(),
            GENERIC_READ,
            0, // exclusive access
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if (hFile == INVALID_HANDLE_VALUE)
            throw std::exception("Can't open file");

        FILE_BASIC_INFO basicInfo;
        if (GetFileInformationByHandleEx(hFile, FileBasicInfo, &basicInfo, sizeof(basicInfo)) == 0)
            throw std::exception("Can't get FileBasicInfo");

        ::CloseHandle(hFile);


        int dir = filename.rfind(L"\\");
        std::wstring path = filename.substr(0, dir + 1);
        std::wstring name = filename.substr(dir + 1);

        std::ifstream inFile;
        inFile.open(filename.c_str(), std::ios::binary);

        inFile.seekg(0, std::ios::end);
        uint64_t fileSize = inFile.tellg();
        inFile.seekg(0, std::ios::beg);
        
        inFile.close();
        return FileHeader{ path, name, fileSize, basicInfo };
    }


    std::vector<std::vector<uint8_t>> FileSystem::ReadFileChunks(std::wstring filename, int chunkSize) {
        std::replace(filename.begin(), filename.end(), L'/', L'\\');

        int dir = filename.rfind(L"\\");
        std::wstring path = filename.substr(0, dir);
        std::wstring name = filename.substr(dir + 1);

        std::ifstream inFile;
        inFile.open(filename.c_str(), std::ios::binary);

        inFile.seekg(0, std::ios::end);
        uint64_t fileSize = inFile.tellg();
        inFile.seekg(0, std::ios::beg);

        if (fileSize <= chunkSize) {
            std::vector<std::vector<uint8_t>> vecChunks(1, std::vector<uint8_t>(fileSize));
            inFile.read((char*)vecChunks.back().data(), fileSize);
            inFile.close();
            return std::move(vecChunks);
        }

        std::vector<std::vector<uint8_t>> vecChunks(std::ceil(float(fileSize) / chunkSize), std::vector<uint8_t>(chunkSize));
        vecChunks.back().resize(fileSize % chunkSize);

        for (auto& chunk : vecChunks) {
            inFile.read((char*)chunk.data(), chunk.size());
        }

        inFile.close();
        return std::move(vecChunks);
    }



    void FileSystem::WriteFileWithHeader(FileHeader fileHeader, const std::vector<uint8_t>& fileData) {
        std::ofstream outFile;

        if (!std::filesystem::exists(fileHeader.path))
            std::filesystem::create_directories(fileHeader.path);

        std::wstring filename = fileHeader.path + fileHeader.name;

        outFile.open(filename, std::ios::binary);
        outFile.write((char*)fileData.data(), fileData.size());
        outFile.close();

        HANDLE hFile = ::CreateFileW(filename.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0, // exclusive access
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if (hFile == INVALID_HANDLE_VALUE) {
            //WriteDebug(L"Can't open file = %s", filename.c_str());
            //LogLastError;
            return;
        }

        if (SetFileInformationByHandle(hFile, FileBasicInfo, &fileHeader.basicInfo, sizeof(FILE_BASIC_INFO)) == 0) {
            //WriteDebug(L"Can't set FileBasicInfo for file = %s", filename.c_str());
            //LogLastError;
        }

        ::CloseHandle(hFile);
    }
}