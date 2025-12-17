#pragma once
#include "FilesCollection.h"

namespace HELPERS_NS {
    namespace FS {
        void FilesCollection::ReplaceRootPath(std::wstring newRootPath) {
            for (auto& dir : dirs) {
                dir = newRootPath / dir.relative_path();
            }
            for (auto& file : files) {
                file = newRootPath / file.relative_path();
            }
        }

        const std::vector<std::filesystem::path>& FilesCollection::GetDirs() const {
            return dirs;
        }

        const std::vector<std::filesystem::path>& FilesCollection::GetFiles() const {
            return files;
        }

        const uint64_t FilesCollection::GetSize() const {
            return totalSize;
        }

        void FilesCollection::Initialize() {
            dirs.clear();
            files.clear();
            totalSize = 0;
        }

        void FilesCollection::HandlePathItem(const PathItem& pathItem) {
            std::filesystem::path pathEntry;

            switch (pathItem.type) {
            case PathItem::Type::File:
            case PathItem::Type::Directory:
                pathEntry = pathItem.mainItem->path;
                break;
            case PathItem::Type::RecursiveEntry:
                pathEntry = pathItem.recursiveItem->path;
                break;
            }

            switch (pathItem.ExpandType()) {
            case PathItem::Type::File:
                files.push_back(pathEntry);
                totalSize += std::filesystem::file_size(pathItem.mainItem->path);
                break;
            case PathItem::Type::Directory:
                dirs.push_back(pathEntry);
                break;
            }
        }

        void FilesCollection::Complete() {
        }
    }
}

std::ostream& operator<< (std::ostream& out, const HELPERS_NS::FS::FilesCollection& filesCollection) {
    for (auto& file : filesCollection.GetFiles()) {
        out << file << "\n";
    }
    return out;
}