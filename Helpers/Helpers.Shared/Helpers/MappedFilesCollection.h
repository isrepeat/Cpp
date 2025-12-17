#pragma once
#include "common.h"
#include "FilesObserver.h"
#include <Helpers/Flags.h>
#include <iostream>

namespace HELPERS_NS {
    namespace FS {
        struct FileItemWithMappedPath : FileItemBase {
            FileItemWithMappedPath() = default;
            ~FileItemWithMappedPath() = default;

            FileItemWithMappedPath(std::string path, std::string mappedPath)
                : FileItemBase{ path }
                , mappedPath{ mappedPath }
            {}
            FileItemWithMappedPath(std::wstring path, std::wstring mappedPath)
                : FileItemBase{ path }
                , mappedPath{ mappedPath }
            {}
            FileItemWithMappedPath(const char* path, const char* mappedPath)
                : FileItemBase{ path }
                , mappedPath{ mappedPath }
            {}
            FileItemWithMappedPath(const wchar_t* path, const wchar_t* mappedPath)
                : FileItemBase{ path }
                , mappedPath{ mappedPath }
            {}
            FileItemWithMappedPath(std::filesystem::path path, std::filesystem::path mappedPath)
                : FileItemBase{ path }
                , mappedPath{ mappedPath }
            {}

            std::filesystem::path mappedPath;
        };

        struct MappedFileItem {
            std::filesystem::path localPath;
            std::filesystem::path mappedPath;
        };

        class MappedFilesCollection : public IFilesCollection {
        public:
            enum Format {
                None = 0x00,
                KeepRelativeMappedPath = 0x01,
                RenameDuplicates = 0x02,
                PreserveDirStructure = 0x04,
                Default = None
            };

            MappedFilesCollection(std::filesystem::path mappedRootPath,
                HELPERS_NS::Flags<Format> formatFlags = Format::Default);

            MappedFilesCollection(MappedFilesCollection&) = default;

            // After copying, internally calls Complete()
            MappedFilesCollection& operator=(const MappedFilesCollection& other);

            bool IsPreserveDirectoryStructure();

            const uint64_t GetSize() const;
            const std::vector<MappedFileItem>& GetDirs() const;
            const std::vector<MappedFileItem>& GetFiles() const;

            HELPERS_NS::Flags<Format> GetFormatFlags() const;

            const MappedFileItem& operator[](int i) const;

        private:
            void Initialize() override;
            void HandlePathItem(const PathItem& pathItem) override;
            void Complete() override;

            void RenameDuplicatesInCollection(std::vector<HELPERS_NS::FS::MappedFileItem>& collection);

        private:
            uint64_t totalSize;
            const HELPERS_NS::Flags<Format> formatFlags;
            std::filesystem::path mappedRootPath;
            std::vector<MappedFileItem> dirs;
            std::vector<MappedFileItem> files;
        };

        void GetFilesCollection(std::vector<FileItemWithMappedPath> fileItems, MappedFilesCollection& filesCollection);
    }
}

std::ostream& operator<< (std::ostream& out, const std::vector<HELPERS_NS::FS::MappedFileItem>& mappedPathItems);
std::ostream& operator<< (std::ostream& out, const HELPERS_NS::FS::MappedFilesCollection& mappedFilesCollection);