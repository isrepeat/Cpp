#pragma once
#include "MappedFilesCollection.h"
#include <Helpers/TypeSwitch.h>

namespace HELPERS_NS {
    namespace FS {
        MappedFilesCollection::MappedFilesCollection(std::filesystem::path mappedRootPath, HELPERS_NS::Flags<Format> formatFlags)
            : totalSize{ 0 }
            , formatFlags{ formatFlags }
            , mappedRootPath{ mappedRootPath }
        {
        }

        MappedFilesCollection& MappedFilesCollection::operator=(const MappedFilesCollection& other) {
            if (this != &other) {
                dirs = other.dirs;
                files = other.files;
                totalSize = other.totalSize;
                mappedRootPath = other.mappedRootPath;
                Complete();
            }
            return *this;
        }

        bool MappedFilesCollection::IsPreserveDirectoryStructure() {
            return formatFlags.Has(Format::PreserveDirStructure);
        }

        const uint64_t MappedFilesCollection::GetSize() const {
            return totalSize;
        }

        const std::vector<MappedFileItem>& MappedFilesCollection::GetDirs() const {
            return dirs;
        }

        const std::vector<MappedFileItem>& MappedFilesCollection::GetFiles() const {
            return files;
        }

        HELPERS_NS::Flags<MappedFilesCollection::Format> MappedFilesCollection::GetFormatFlags() const {
            return formatFlags;
        }

        const MappedFileItem& MappedFilesCollection::operator[](int i) const {
            return files[i];
        }


        void MappedFilesCollection::Initialize() {
            dirs.clear();
            files.clear();
            totalSize = 0;
        }

        void MappedFilesCollection::HandlePathItem(const PathItem& pathItem) {
            MappedFileItem mappedFileItem;
            auto basePath = mappedRootPath;
            auto fileName = pathItem.mainItem->path.filename();

            if (formatFlags.Has(Format::PreserveDirStructure)) {
                basePath /= pathItem.mainItem->path.relative_path().remove_filename();
            }

            HELPERS_NS::TypeSwitch(pathItem.mainItem.get(),
                [&basePath](FileItemWithMappedPath& item) {
                    basePath /= item.mappedPath.relative_path().remove_filename();
                    return;
                }
            );

            switch (pathItem.type) {
            case PathItem::Type::File:
            case PathItem::Type::Directory:
                mappedFileItem = { pathItem.mainItem->path, basePath / fileName};
                break;
            case PathItem::Type::RecursiveEntry: {
                // Cut mainItem path part from recursiveItem (recursiveItem is a child item of mainItem)
                auto relativePathToMainItem = std::filesystem::relative(pathItem.recursiveItem->path, pathItem.mainItem->path);
                mappedFileItem = { pathItem.recursiveItem->path, basePath / fileName / relativePathToMainItem };
                break;
            }
            }

            switch (pathItem.ExpandType()) {
            case PathItem::Type::File:
                files.push_back(std::move(mappedFileItem));
                totalSize += std::filesystem::file_size(pathItem.mainItem->path);
                break;
            case PathItem::Type::Directory:
                dirs.push_back(std::move(mappedFileItem));
                break;
            }
        }

        void MappedFilesCollection::Complete() {
            if (formatFlags.Has(Format::RenameDuplicates)) {
                // Rename only duplicate files, "duplicate" dirs will be merged
                RenameDuplicatesInCollection(files);
            }

            if (formatFlags.Has(Format::KeepRelativeMappedPath)) { // remove root path
                for (auto& dir : dirs) {
                    dir.mappedPath = dir.mappedPath.relative_path();
                }
                for (auto& file : files) {
                    file.mappedPath = file.mappedPath.relative_path();
                }
            }
        }

        void MappedFilesCollection::RenameDuplicatesInCollection(std::vector<HELPERS_NS::FS::MappedFileItem>& collection) {
            std::vector<std::filesystem::path> existingFiles;
            HELPERS_NS::FS::GetAllFiles(this->mappedRootPath, existingFiles);

            // Add files that already exist at the mapped path to the collection
            if (!existingFiles.empty()) {
                collection.reserve(collection.size() + existingFiles.size());
                for (auto& path : existingFiles) {
                    collection.push_back({"", path});
                }

                // Put existing files to the beginning of the list, as duplicate renaming works back to front
                std::rotate(collection.begin(), collection.end() - existingFiles.size(), collection.end());
            }

            for (auto itemIt = collection.rbegin(); itemIt != collection.rend(); ++itemIt) {
                auto pathTmp = std::move(itemIt->mappedPath); // Temporarily "remove" from list
                HELPERS_NS::FS::RenameDuplicate(pathTmp, collection, [&](const MappedFileItem& item) {
                    return item.mappedPath == pathTmp;
                    });
                itemIt->mappedPath = pathTmp;
            }

            // Remove existing files from the collection
            collection.erase(collection.begin(), collection.begin() + existingFiles.size());
        }

        void GetFilesCollection(std::vector<FileItemWithMappedPath> fileItems, MappedFilesCollection& filesCollection) {
            std::vector<std::unique_ptr<FileItemBase>> fileItemsUniq;

            std::transform(fileItems.begin(), fileItems.end(), std::back_inserter(fileItemsUniq),
                [](const FileItemWithMappedPath& item) {
                    return std::make_unique<FileItemWithMappedPath>(item);
                });

            GetFilesCollection<MappedFilesCollection>(std::move(fileItemsUniq), filesCollection);
        }
    }
}

std::ostream& operator<< (std::ostream& out, const std::vector<HELPERS_NS::FS::MappedFileItem>& mappedPathItems) {
    for (auto& file : mappedPathItems) {
        out << file.localPath << "\n";
        out << file.mappedPath << "\n\n";
    }
    return out;
}

std::ostream& operator<< (std::ostream& out, const HELPERS_NS::FS::MappedFilesCollection& mappedFilesCollection) {
    for (auto& file : mappedFilesCollection.GetFiles()) {
        out << file.localPath << "\n";
        out << file.mappedPath << "\n\n";
    }
    return out;
}