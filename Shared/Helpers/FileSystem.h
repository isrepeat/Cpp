#pragma once
#include "HWindows.h"
#include "Filesystem.inl"
#include <functional>
#include <vector>
#include <string>

namespace H {
    namespace FS {
        struct FileHeader {
            std::wstring path;
            std::wstring name;
            uint64_t filesize;
            FILE_BASIC_INFO basicInfo;
        };


        bool RemoveFile(const std::wstring& filename);
        void RenameFile(const std::wstring& oldFilename, const std::wstring& newFilename);

        std::vector<uint8_t> ReadFile(const std::wstring& filename);
        void WriteFile(const std::wstring& filename, const std::vector<uint8_t>& fileData);

        FileHeader ReadFileHeader(std::wstring filename);
        std::vector<std::vector<uint8_t>> ReadFileChunks(std::wstring filename, int chunkSize);

        void WriteFileWithHeader(FileHeader fileHeader, const std::vector<uint8_t>& fileData);
    }
}