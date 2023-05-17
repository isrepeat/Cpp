#pragma once
#include <Windows.h>
#include <vector>
#include <string>

namespace H {
    struct FileHeader {
        std::wstring path;
        std::wstring name;
        uint64_t filesize;
        FILE_BASIC_INFO basicInfo;
    };


    class FileSystem {
    public:
        static bool RemoveFile(const std::wstring& filename);
        static void RenameFile(const std::wstring& oldFilename, const std::wstring& newFilename);
        static std::vector<uint8_t> ReadFile(const std::wstring& filename);
        static void WriteFile(const std::wstring& filename, const std::vector<uint8_t>& fileData);
        
        static FileHeader ReadFileHeader(std::wstring filename);
        static std::vector<std::vector<uint8_t>> ReadFileChunks(std::wstring filename, int chunkSize);
        
        static void WriteFileWithHeader(FileHeader fileHeader, const std::vector<uint8_t>& fileData);
    };
}