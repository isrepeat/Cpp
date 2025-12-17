#pragma once
#include "common.h"
#include <filesystem>
#include <cassert>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>

namespace HELPERS_NS {
    namespace FS {
        struct FileItemBase {
            FileItemBase() = default;
            virtual ~FileItemBase() = default;

            FileItemBase(std::string path) : path{ path } {}
            FileItemBase(std::wstring path) : path{ path } {}
            FileItemBase(const char* path) : path{ path } {}
            FileItemBase(const wchar_t* path) : path{ path } {}
            FileItemBase(std::filesystem::path path) : path{ path } {}
            
            std::filesystem::path path;
        };

        struct PathItem {
            enum class Type {
                File,
                Directory,
                RecursiveEntry,
            };

            PathItem(Type type, std::unique_ptr<FileItemBase> mainItem)
                : type{ type }
                , mainItem{ std::move(mainItem) }
            {}

            Type ExpandType() const {
                if (type == PathItem::Type::RecursiveEntry) {
                    assert(!recursiveItem->path.empty() && "--> recursiveItem is empty!");

                    if (std::filesystem::is_regular_file(recursiveItem->path)) {
                        return Type::File;
                    }
                    else if (std::filesystem::is_directory(recursiveItem->path)) {
                        return Type::Directory;
                    }
                }
                return type;
            }

            Type type;
            std::unique_ptr<FileItemBase> mainItem;
            std::unique_ptr<FileItemBase> recursiveItem;
        };


        class IFilesCollection {
        public:
            virtual ~IFilesCollection() = default;

        protected:
            template <typename FilesCollectionT>
            friend void GetFilesCollection(std::vector<std::unique_ptr<FileItemBase>>, FilesCollectionT&);

            template <typename FileItemT, typename FilesCollectionT>
            friend void GetFilesCollection(std::vector<FileItemT>, FilesCollectionT&);


            virtual void Initialize() = 0;
            virtual void HandlePathItem(const PathItem& pathItem) = 0;
            virtual void Complete() = 0;
        };


        template <typename FilesCollectionT>
        void GetFilesCollection(std::vector<std::unique_ptr<FileItemBase>> fileItemsUniq, FilesCollectionT& filesCollection) {
            IFilesCollection& filesCollectionInterface = filesCollection;
            filesCollectionInterface.Initialize();

            for (auto& item : fileItemsUniq) {
                if (std::filesystem::is_regular_file(item->path)) {
                    filesCollectionInterface.HandlePathItem(PathItem{ PathItem::Type::File, std::move(item) });
                }
                else if (std::filesystem::is_directory(item->path)) {
                    PathItem pathItem{ PathItem::Type::Directory, std::move(item) };
                    filesCollectionInterface.HandlePathItem(pathItem);

                    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(pathItem.mainItem->path)) {
                        pathItem.type = PathItem::Type::RecursiveEntry;
                        pathItem.recursiveItem = std::make_unique<FileItemBase>(dirEntry.path());
                        filesCollectionInterface.HandlePathItem(pathItem);
                    }
                }
            }
            filesCollectionInterface.Complete();
        }

        // Base implementation for convertion from FileItemT to std::unique_ptr<FileItemBase>
        template <typename FileItemT, typename FilesCollectionT>
        void GetFilesCollection(std::vector<FileItemT> fileItems, FilesCollectionT& filesCollection) {
            std::vector<std::unique_ptr<FileItemBase>> fileItemsUniq;

            std::transform(fileItems.begin(), fileItems.end(), std::back_inserter(fileItemsUniq),
                [](const FileItemT& item) {
                    return std::make_unique<FileItemBase>(item);
                });

            GetFilesCollection<FilesCollectionT>(std::move(fileItemsUniq), filesCollection);
        }

        template<template<class> class TCollection, class... Args>
        static void GetAllFiles(const std::filesystem::path& basePath, TCollection<Args...>& collection) {
            try {
                for (auto fileEntry : std::filesystem::recursive_directory_iterator(basePath)) {
                    collection.insert(collection.end(), fileEntry.path());
                }
            }
            catch (...) {
                // Ignore
            }
        }


        template<template<class> class TCollection, class... Args>
        static void RenameDuplicate(
            std::filesystem::path& path,
            const TCollection<Args...>& collection,
            std::function<bool(const typename TCollection<Args...>::value_type&)> predicate) 
        {
            auto duplicates = std::count_if(collection.begin(), collection.end(), predicate);

            if (duplicates > 0) {
                auto nameSuffix = L" (" + std::to_wstring(duplicates) + L")";
                auto ext = path.extension();
                path = path.replace_extension().wstring() + nameSuffix + ext.wstring();
            }
        }
    }
}