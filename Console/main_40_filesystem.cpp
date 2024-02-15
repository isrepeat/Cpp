#include <Helpers/MappedFilesCollection.h>
#include <Helpers/FilesCollection.h>
#include <Helpers/Helpers.h>
#include <Windows.h>
#include <filesystem>
#include <iostream>
#include <cassert>
#include <string>


const std::filesystem::path rootDir = H::ExePath() / "\\TestFilesystem\\";

/* ---------------------------------------------- */
/*   TestCopyDirectoryContentToAnotherDirectory   */
/* ---------------------------------------------- */
void TestCopyDirectoryContentToAnotherDirectory() {
    std::filesystem::path fromDir = rootDir / "Logs";
    std::filesystem::path toDir = rootDir / "CrashReport";

    if (!std::filesystem::exists(fromDir)) {
        throw std::exception("dir not exist");
    }

    if (!std::filesystem::exists(toDir)) {
        std::filesystem::create_directory(toDir);
    }

    std::filesystem::copy(fromDir, toDir, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
    return;
}


/* ------------------ */
/*   TestPathToFile   */
/* ------------------ */
void TestPathToFile() {
    std::wstring filename = L"D:\\WORK\\TEST\\AAA\\BBB\\foo.txt";
    std::wstring folder = std::filesystem::path(filename).parent_path();
    return;
}


/* ------------------ */
/*   TestVolumeInfo   */
/* ------------------ */
void TestVolumeInfo() {
    wchar_t szVolumeName[100] = L"";
    wchar_t szFileSystemName[10] = L"";
    DWORD dwSerialNumber = 0;
    DWORD dwMaxFileNameLength = 0;
    DWORD dwFileSystemFlags = 0;

    if (!GetVolumeInformationW(L"D:\\", 
        szVolumeName, 
        sizeof(szVolumeName),
        &dwSerialNumber,
        &dwMaxFileNameLength,
        &dwFileSystemFlags,
        szFileSystemName, 
        sizeof(szFileSystemName)))
    {
        int fail = 1;
    }

    struct VolumeInfo {
        std::wstring volumeName;
        std::wstring fileSystemName;
    };

    VolumeInfo volumeInfo;
    volumeInfo.volumeName = szVolumeName;
    volumeInfo.fileSystemName = szFileSystemName;

    return;
}



/* ----------------------- */
/*   TestFilesCollection   */
/* ----------------------- */
//struct PathItem {
//    enum class Type {
//        File,
//        Directory,
//        RecursiveEntry,
//    };
//
//    Type ExpandType() const {
//        if (type == PathItem::Type::RecursiveEntry) {
//            assert(!recursiveItem.empty() && "--> recursiveItem is empty!");
//
//            if (std::filesystem::is_regular_file(recursiveItem)) {
//                return Type::File;
//            }
//            else if (std::filesystem::is_directory(recursiveItem)) {
//                return Type::Directory;
//            }
//        }
//        return type;
//    }
//
//    Type type;
//    std::filesystem::path mainItem;
//    std::filesystem::path recursiveItem;
//};
//
//class IFilesCollection {
//public:
//    virtual ~IFilesCollection() = default;
//
//protected:
//    friend class FilesObserver;
//    virtual void Initialize() = 0;
//    virtual void HandlePathItem(const PathItem& pathItem) = 0;
//    virtual void Complete() = 0;
//};
//
//
//class FilesObserver {
//private:
//    FilesObserver() = delete;
//    ~FilesObserver() = delete;
//
//public:
//    template <typename TCollection>
//    static void GetFilesCollection(const std::vector<std::filesystem::path>& filePaths, TCollection& filesCollection) {
//        IFilesCollection& filesCollectionInterface = filesCollection;
//        filesCollectionInterface.Initialize();
//
//        for (auto& item : filePaths) {
//            if (std::filesystem::is_regular_file(item)) {
//                filesCollectionInterface.HandlePathItem(PathItem{ PathItem::Type::File, std::move(item) });
//            }
//            else if (std::filesystem::is_directory(item)) {
//                PathItem pathItem{ PathItem::Type::Directory, std::move(item) };
//                filesCollectionInterface.HandlePathItem(pathItem);
//
//                for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(item)) {
//                    pathItem.type = PathItem::Type::RecursiveEntry;
//                    pathItem.recursiveItem = dirEntry.path();
//                    filesCollectionInterface.HandlePathItem(pathItem);
//                }
//            }
//        }
//        filesCollectionInterface.Complete();
//    };
//
//
//    template <typename TCollection = FilesCollection>
//    static TCollection GetFilesCollection(const std::vector<std::filesystem::path>& filePaths) {
//        TCollection filesCollection; // TCollection must have default Ctor
//        GetFilesCollection<TCollection>(filePaths, filesCollection);
//        return filesCollection;
//    };
//};
//
//
//
//class FilesCollection : public IFilesCollection {
//public:
//    FilesCollection() = default;
//
//    void ReplaceRootPath(std::wstring newRootPath) {
//        for (auto& dir : dirs) {
//            dir = newRootPath / dir.relative_path();
//        }
//        for (auto& file : files) {
//            file = newRootPath / file.relative_path();
//        }
//    }
//
//    const std::vector<std::filesystem::path>& GetDirs() const {
//        return dirs;
//    }
//
//    const std::vector<std::filesystem::path>& GetFiles() const {
//        return files;
//    }
//
//    const uint64_t GetSize() const {
//        return totalSize;
//    }
//
//private:
//    void Initialize() override {
//        dirs.clear();
//        files.clear();
//        totalSize = 0;
//    }
//
//    void HandlePathItem(const PathItem& pathItem) override {
//        switch (pathItem.ExpandType()) {
//        case PathItem::Type::File:
//            files.push_back(pathItem.mainItem);
//            totalSize += std::filesystem::file_size(pathItem.mainItem);
//            break;
//        case PathItem::Type::Directory:
//            dirs.push_back(pathItem.mainItem);
//            break;
//        }
//    }
//
//    void Complete() override {
//    }
//
//private:
//    uint64_t totalSize = 0;
//    std::vector<std::filesystem::path> dirs;
//    std::vector<std::filesystem::path> files;
//};
//
//
//
//struct MappedFileItem {
//    std::filesystem::path localPath;
//    std::filesystem::path mappedPath;
//};
//
//class MappedFilesCollection : public IFilesCollection {
//public:
//    enum class Format {
//        Default,
//        RelativeMappedPath,
//    };
//
//    MappedFilesCollection(std::filesystem::path mappedRootPath, Format format = Format::Default)
//        : totalSize{ 0 }
//        , format{ format }
//        , mappedRootPath{ mappedRootPath }
//    {}
//    MappedFilesCollection(MappedFilesCollection&) = default;
//
//    // After copying, internally calls Complete()
//    MappedFilesCollection& operator=(const MappedFilesCollection& other) {
//        if (this != &other) {
//            dirs = other.dirs;
//            files = other.files;
//            totalSize = other.totalSize;
//            mappedRootPath = other.mappedRootPath;
//            Complete();
//        }
//        return *this;
//    }
//
//    const std::vector<MappedFileItem>& GetDirs() const {
//        return dirs;
//    }
//    const std::vector<MappedFileItem>& GetFiles() const {
//        return files;
//    }
//    const uint64_t GetSize() const {
//        return totalSize;
//    }
//
//private:
//    void Initialize() override {
//        dirs.clear();
//        files.clear();
//        totalSize = 0;
//    }
//
//    void HandlePathItem(const PathItem& pathItem) override {
//        MappedFileItem mappedFileItem;
//
//        switch (pathItem.type) {
//        case PathItem::Type::File:
//        case PathItem::Type::Directory:
//            mappedFileItem = { pathItem.mainItem, mappedRootPath / pathItem.mainItem.filename() };
//            break;
//        case PathItem::Type::RecursiveEntry: {
//            // Cut mainItem path part from recursiveItem (recursiveItem is a child item of mainItem)
//            auto relativePathToMainItem = pathItem.recursiveItem.wstring().substr(pathItem.mainItem.wstring().size() + 1); // +1 - skip slash
//            mappedFileItem = { pathItem.recursiveItem, mappedRootPath / pathItem.mainItem.filename() / relativePathToMainItem };
//            break;
//        }
//        }
//
//        switch (pathItem.ExpandType()) {
//        case PathItem::Type::File:
//            files.push_back(std::move(mappedFileItem));
//            totalSize += std::filesystem::file_size(pathItem.mainItem);
//            break;
//        case PathItem::Type::Directory:
//            dirs.push_back(std::move(mappedFileItem));
//            break;
//        }
//    }
//
//    void Complete() override {
//        if (format == Format::RelativeMappedPath) { // remove root path
//            for (auto& dir : dirs) {
//                dir.mappedPath = dir.mappedPath.relative_path();
//            }
//            for (auto& file : files) {
//                file.mappedPath = file.mappedPath.relative_path();
//            }
//        }
//    }
//
//private:
//    uint64_t totalSize;
//    const Format format;
//    std::filesystem::path mappedRootPath;
//    std::vector<MappedFileItem> dirs;
//    std::vector<MappedFileItem> files;
//};



void TestFilesCollection() {    
    //std::filesystem::path tmpPath = L"C:\\TORRENT_TMP\\Hello\\";
    std::filesystem::path tmpPath = L"\\TORRENT_TMP\\Hello";
    auto rootPath = tmpPath.root_path();
    auto relPath = tmpPath.relative_path();
    auto filename = tmpPath.filename();

    std::vector<std::filesystem::path> fileList = {
        L"C:\\TORRENT_TMP\\Hello\\sockets.pdf",
        L"C:\\TORRENT_TMP\\Hello\\Folder_C",
        L"D:\\WORK\\TEST\\TeamRemoteDesktop\\Clipboard\\Sender\\Atribute_Perspective_(62).png",
        L"D:\\WORK\\TEST\\TeamRemoteDesktop\\Clipboard\\Sender\\Folder_D",
        //L"D:\\black.png",
    };


    auto filesCollection_implicit = H::FS::GetFilesCollection(fileList);
    std::cout << "filesCollection_implicit: \n" << filesCollection_implicit << "\n";

    H::FS::FilesCollection filesCollection_explicit;
    H::FS::GetFilesCollection(fileList, filesCollection_explicit);
    std::cout << "filesCollection_explicit: \n" << filesCollection_explicit << "\n";

    //MappedFilesCollection mappedFilesCollection_explicit("X:\\ROOT");
    H::FS::MappedFilesCollection mappedFilesCollection_explicit("X:\\");
    H::FS::GetFilesCollection(fileList, mappedFilesCollection_explicit);
    std::cout << "mappedFilesCollection_explicit: \n" << mappedFilesCollection_explicit << "\n";

    system("pause");
    return;
}


void TestMappedFilesCollectionWithSelfMappedPaths() {
    //std::vector<std::filesystem::path> fileList = {
    std::vector<H::FS::FileItemWithMappedPath> fileList = {
        {L"D:\\WORK\\TEST\\FILE_SYSTEM", "" },
        { L"D:\\WORK\\TEST\\FILE_SYSTEM\\sockets.pdf", "__AA__\\__BB__" },
        { L"C:\\TORRENT_TMP\\Hello\\sockets.pdf", "__AA__\\__BB__" },
    };

    H::Flags<H::FS::MappedFilesCollection::Format> mappedFilesCollectionFormat =
        H::FS::MappedFilesCollection::RenameDuplicates |
        H::FS::MappedFilesCollection::PreserveDirStructure;

    H::FS::MappedFilesCollection mappedFilesCollection("X:\\DiscFolder", mappedFilesCollectionFormat);
    H::FS::GetFilesCollection(fileList, mappedFilesCollection);
    std::cout << "mappedFilesCollection->GetDirs(): \n" << mappedFilesCollection.GetDirs() << "\n\n";
    std::cout << "mappedFilesCollection->GetFiles(): \n" << mappedFilesCollection.GetFiles() << "\n\n";

    system("pause");
    return;
}



void main() {
    //TestCopyDirectoryContentToAnotherDirectory();
    //TestPathToFile();
    //TestVolumeInfo();
    //TestFilesCollection();
    TestMappedFilesCollectionWithSelfMappedPaths();

    return;
}