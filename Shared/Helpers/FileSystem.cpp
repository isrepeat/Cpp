#include "FileSystem.h"
#include "Helpers.h"
//#include "Logger.h"
#include <filesystem>
#include <algorithm>
#include <fstream>

#define LogDebugWithFullClassNameA(...)
#define LogDebugWithFullClassNameW(...)

#define LogWarningWithFullClassNameA(...)
#define LogWarningWithFullClassNameW(...)

#define LogErrorWithFullClassNameA(...)
#define LogErrorWithFullClassNameW(...)

#define LOG_FUNCTION_ENTER(...)

#define LOG_DEBUG(...)
#define LOG_ERROR(...)

#define LOG_DEBUG_D(...)
#define LOG_ERROR_D(...)

#define LogLastError

namespace H {
    namespace FS {
        bool RemoveFile(const std::wstring& filename) {
            return std::filesystem::remove(filename);
        }

        void RenameFile(const std::wstring& oldFilename, const std::wstring& newFilename) {
            return std::filesystem::rename(oldFilename, newFilename);
        }

        std::vector<uint8_t> ReadFile(const std::wstring& filename) {
            // TODO: add auto find filesize
            std::vector<uint8_t> buff(512, '\0');
            std::ifstream inFile;
            inFile.open(filename, std::ios::binary);
            inFile.read((char*)buff.data(), 512);
            inFile.close();

            return std::move(buff);
        }

        void WriteFile(const std::wstring& filename, const std::vector<uint8_t>& fileData) {
            std::ofstream outFile;
            outFile.open(filename, std::ios::binary);
            outFile.write((char*)fileData.data(), fileData.size());
            outFile.close();
        }


        FileHeader ReadFileHeader(std::wstring filename) {
            std::replace(filename.begin(), filename.end(), L'/', L'\\');

            HANDLE hFile = ::CreateFileW(filename.c_str(),
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

            if (hFile == INVALID_HANDLE_VALUE)
                throw std::exception(("Can't open file: " + H::WStrToStr(filename)).c_str());

            FILE_BASIC_INFO basicInfo;
            if (GetFileInformationByHandleEx(hFile, FileBasicInfo, &basicInfo, sizeof(basicInfo)) == 0)
                throw std::exception(("Can't get FileBasicInfo: " + H::WStrToStr(filename)).c_str());

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


        std::vector<std::vector<uint8_t>> ReadFileChunks(std::wstring filename, int chunkSize) {
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



        void WriteFileWithHeader(FileHeader fileHeader, const std::vector<uint8_t>& fileData) {
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
                LOG_DEBUG(L"Can't open file = {}", filename);
                LogLastError;
                return;
            }

            if (SetFileInformationByHandle(hFile, FileBasicInfo, &fileHeader.basicInfo, sizeof(FILE_BASIC_INFO)) == 0) {
                LOG_DEBUG(L"Can't set FileBasicInfo for file = {}", filename);
                LogLastError;
            }

            ::CloseHandle(hFile);
        }
    }
}