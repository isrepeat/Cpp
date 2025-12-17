#include "FileSystem.h"
#if COMPILE_FOR_DESKTOP || COMPILE_FOR_CX
#include "Helpers.h"
#include "Logger.h"
#include "Scope.h"
#include <filesystem>
#include <algorithm>
#include <fstream>

namespace HELPERS_NS {
    namespace FS {
        bool RemoveFile(const std::wstring& filename) {
            return std::filesystem::remove(filename);
        }

        void RenameFile(const std::wstring& oldFilename, const std::wstring& newFilename) {
            return std::filesystem::rename(oldFilename, newFilename);
        }

        /* ------------------- */
        /*     Read / Write    */
        /* ------------------- */
        std::vector<uint8_t> ReadFile(const std::filesystem::path& filename) {
            std::vector<uint8_t> buffer;
            buffer.resize(std::filesystem::file_size(filename));

            std::ifstream inFile;
            inFile.open(filename, std::ios::binary);
            inFile.read((char*)buffer.data(), buffer.size());
            inFile.close();

            return buffer;
        }

        void WriteFile(const std::filesystem::path& filename, const std::vector<uint8_t>& fileData) {
            std::ofstream outFile;
            outFile.open(filename, std::ios::binary);
            outFile.write((char*)fileData.data(), fileData.size());
            outFile.close();
        }


#if COMPILE_FOR_DESKTOP
        FileHeader ReadFileHeader(const std::filesystem::path& filename) {
            HANDLE hFile = ::CreateFileW(filename.c_str(),
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

            auto closeFileHandleScoped = HELPERS_NS::MakeScope([hFile] {
                ::CloseHandle(hFile);
                });

            if (hFile == INVALID_HANDLE_VALUE) {
                LOG_THROW_STD_EXCEPTION("Can't open file = {}", filename);
            }

            FILE_BASIC_INFO basicInfo;
            if (GetFileInformationByHandleEx(hFile, FileBasicInfo, &basicInfo, sizeof(basicInfo)) == FALSE) {
                LOG_THROW_STD_EXCEPTION("Can't get FileBasicInfo: {}", filename);
            }

            uint64_t fileSize = std::filesystem::file_size(filename);
            return FileHeader{ filename, basicInfo, fileSize };
        }


        std::vector<std::vector<uint8_t>> ReadFileChunks(const std::filesystem::path& filename, int chunkSize) {
            std::ifstream inFile;
            inFile.open(filename, std::ios::binary);

            uint64_t fileSize = std::filesystem::file_size(filename);

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
            if (!std::filesystem::exists(fileHeader.fileItem))
                std::filesystem::create_directories(fileHeader.fileItem.parent_path());

            std::ofstream outFile;
            outFile.open(fileHeader.fileItem, std::ios::binary);
            outFile.write((char*)fileData.data(), fileData.size());
            outFile.close();

            HANDLE hFile = ::CreateFileW(fileHeader.fileItem.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                0, // exclusive access
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

            auto closeFileHandleScoped = HELPERS_NS::MakeScope([hFile] {
                ::CloseHandle(hFile);
                });

            if (hFile == INVALID_HANDLE_VALUE) {
                LOG_DEBUG(L"Can't open file = {}", fileHeader.fileItem);
                LogLastError;
                return;
            }

            if (SetFileInformationByHandle(hFile, FileBasicInfo, &fileHeader.basicInfo, sizeof(FILE_BASIC_INFO)) == 0) {
                LOG_DEBUG(L"Can't set FileBasicInfo for file = {}", fileHeader.fileItem);
                LogLastError;
                return;
            }
        }
#endif

        /* ------------------- */
        /*     Path helpers    */
        /* ------------------- */
        void ConvertPathsWithPreferredSlashes(std::vector<std::filesystem::path>& paths) {
            for (auto& path : paths) {
                path.make_preferred();
            }
        }

        /* ------------------- */
        /*    Copy / Replace   */
        /* ------------------- */
        void CopyFirstItem(const std::filesystem::path& fromDir, const std::filesystem::path& toDir, const std::wstring& prefix) {
            if (!std::filesystem::exists(fromDir)) {
                LOG_ERROR_D(L"fromDir not exist [{}], return", fromDir);
                return;
            }
            if (!std::filesystem::exists(toDir)) {
                std::filesystem::create_directory(toDir);
            }
            auto it = std::filesystem::directory_iterator(fromDir);
            const auto& entryPath = it->path();
            std::filesystem::copy(entryPath, toDir / (prefix + entryPath.filename().wstring()), std::filesystem::copy_options::overwrite_existing);
        }

        void CopyDirContentTo(const std::filesystem::path& fromDir, const std::filesystem::path& toDir) {
            if (!std::filesystem::exists(fromDir)) {
                LOG_ERROR_D(L"fromDir not exist [{}], return", fromDir);
                return;
            }
            if (!std::filesystem::exists(toDir)) {
                std::filesystem::create_directory(toDir);
            }
            // NOTE: if you also want to copy subfolder add flag std::filesystem::copy_options::recursive
            std::filesystem::copy(fromDir, toDir, std::filesystem::copy_options::overwrite_existing);
        }

        /* ------------------- */
        /*   Filesystem info   */
        /* ------------------- */
#if COMPILE_FOR_DESKTOP
        std::vector<std::filesystem::path> GetAllLogicalDrives() {
            auto endSize = GetLogicalDriveStringsW(0, nullptr);
            std::wstring disksStr;
            disksStr.resize(endSize);
            GetLogicalDriveStringsW(disksStr.size(), const_cast<wchar_t*>(disksStr.c_str()));

            std::vector<std::filesystem::path> result;
            std::size_t currentLen = 0;
            for (std::size_t i = 0; i < endSize; ++i) {
                if (disksStr[i] == L'\0' && currentLen > 0) {
                    result.push_back(disksStr.substr(i - currentLen, currentLen));
                    currentLen = 0;
                    continue;
                }
                ++currentLen;
            }
            return result;
        }
#endif
    }
}
#endif