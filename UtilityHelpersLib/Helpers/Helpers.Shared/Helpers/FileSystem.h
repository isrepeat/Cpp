#pragma once
#include "common.h"
#if COMPILE_FOR_DESKTOP || COMPILE_FOR_CX
#include "HWindows.h"
#include "FileSystem_inline.h"
#include "File.h"

#include <functional>
#include <vector>
#include <string>

namespace HELPERS_NS {
    namespace FS {
#if COMPILE_FOR_DESKTOP
        struct FileHeader {
            std::filesystem::path fileItem;
            FILE_BASIC_INFO basicInfo;
            uint64_t fileSize;
        };
#endif

        bool RemoveFile(const std::wstring& filename);
        void RenameFile(const std::wstring& oldFilename, const std::wstring& newFilename);

        /* ------------------- */
        /*     Read / Write    */
        /* ------------------- */
        std::vector<uint8_t> ReadFile(const std::filesystem::path& filename);
        void WriteFile(const std::filesystem::path& filename, const std::vector<uint8_t>& fileData);

#if COMPILE_FOR_DESKTOP
        FileHeader ReadFileHeader(const std::filesystem::path& filename);
        std::vector<std::vector<uint8_t>> ReadFileChunks(const std::filesystem::path&, int chunkSize);

        void WriteFileWithHeader(FileHeader fileHeader, const std::vector<uint8_t>& fileData);
#endif

        /* ------------------- */
        /*     Path helpers    */
        /* ------------------- */
        void ConvertPathsWithPreferredSlashes(std::vector<std::filesystem::path>& paths);

        /* ------------------- */
        /*    Copy / Replace   */
        /* ------------------- */
        void CopyFirstItem(const std::filesystem::path& fromDir, const std::filesystem::path& toDir, const std::wstring& prefix = L"");
        void CopyDirContentTo(const std::filesystem::path& fromDir, const std::filesystem::path& toDir);

        /* ------------------- */
        /*   Filesystem info   */
        /* ------------------- */
#if COMPILE_FOR_DESKTOP
        std::vector<std::filesystem::path> GetAllLogicalDrives();
#endif
    }
}
#endif