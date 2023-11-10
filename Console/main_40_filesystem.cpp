//#include <Windows.h>
//#include <Helpers/Helpers.h>
//#include <filesystem>
//#include <iostream>
//#include <cassert>
//#include <string>
//
//
//const std::wstring rootDir = H::ExePathW() + L"\\TestFilesystem\\";
//
///* ---------------------------------------------- */
///*   TestCopyDirectoryContentToAnotherDirectory   */
///* ---------------------------------------------- */
//void TestCopyDirectoryContentToAnotherDirectory() {
//    std::wstring fromDir = rootDir + L"Logs";
//    std::wstring toDir = rootDir +  L"CrashReport";
//
//    if (!std::filesystem::exists(fromDir)) {
//        throw std::exception("dir not exist");
//    }
//
//    if (!std::filesystem::exists(toDir)) {
//        std::filesystem::create_directory(toDir);
//    }
//
//    std::filesystem::copy(fromDir, toDir, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
//    return;
//}
//
//
///* ------------------ */
///*   TestPathToFile   */
///* ------------------ */
//void TestPathToFile() {
//    std::wstring filename = L"D:\\WORK\\TEST\\AAA\\BBB\\foo.txt";
//    std::wstring folder = std::filesystem::path(filename).parent_path();
//    return;
//}
//
//
///* ------------------ */
///*   TestVolumeInfo   */
///* ------------------ */
//void TestVolumeInfo() {
//    wchar_t szVolumeName[100] = L"";
//    wchar_t szFileSystemName[10] = L"";
//    DWORD dwSerialNumber = 0;
//    DWORD dwMaxFileNameLength = 0;
//    DWORD dwFileSystemFlags = 0;
//
//    if (!GetVolumeInformationW(L"D:\\", 
//        szVolumeName, 
//        sizeof(szVolumeName),
//        &dwSerialNumber,
//        &dwMaxFileNameLength,
//        &dwFileSystemFlags,
//        szFileSystemName, 
//        sizeof(szFileSystemName)))
//    {
//        int fail = 1;
//    }
//
//    struct VolumeInfo {
//        std::wstring volumeName;
//        std::wstring fileSystemName;
//    };
//
//    VolumeInfo volumeInfo;
//    volumeInfo.volumeName = szVolumeName;
//    volumeInfo.fileSystemName = szFileSystemName;
//
//    return;
//}
//
//
//
//
///* ----------------------- */
///*   TestFilesCollection   */
///* ----------------------- */
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
//
//
//struct DiscFileItem {
//    std::filesystem::path localPath;
//    std::filesystem::path discPath;
//};
//
//class DiscFilesCollection : public IFilesCollection {
//public:
//    enum class Format {
//        Default,
//        RelativeDiscPath,
//    };
//
//    DiscFilesCollection(wchar_t discLetter, Format format = Format::Default)
//        : totalSize{ 0 }
//        , format{ format }
//        , discRootPath{ discLetter + std::wstring(L":\\") }
//    {}
//    DiscFilesCollection(DiscFilesCollection&) = default;
//
//    // After copying, internally calls Complete()
//    DiscFilesCollection& operator=(const DiscFilesCollection& other) {
//        if (this != &other) {
//            dirs = other.dirs;
//            files = other.files;
//            totalSize = other.totalSize;
//            discRootPath = other.discRootPath;
//            Complete();
//        }
//        return *this;
//    }
//
//    const std::vector<DiscFileItem>& GetDirs() const {
//        return dirs;
//    }
//    const std::vector<DiscFileItem>& GetFiles() const {
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
//        DiscFileItem discFileItem;
//
//        switch (pathItem.type) {
//        case PathItem::Type::File:
//        case PathItem::Type::Directory:
//            discFileItem = { pathItem.mainItem, discRootPath / pathItem.mainItem.filename() };
//            break;
//        case PathItem::Type::RecursiveEntry: {
//            // Cut mainItem path part from recursiveItem (recursiveItem is a child item of mainItem)
//            auto relativePathToMainItem = pathItem.recursiveItem.wstring().substr(pathItem.mainItem.wstring().size() + 1); // +1 - skip slash
//            discFileItem = { pathItem.recursiveItem, discRootPath / pathItem.mainItem.filename() / relativePathToMainItem };
//            break;
//        }
//        }
//
//        switch (pathItem.ExpandType()) {
//        case PathItem::Type::File:
//            files.push_back(std::move(discFileItem));
//            totalSize += std::filesystem::file_size(pathItem.mainItem);
//            break;
//        case PathItem::Type::Directory:
//            dirs.push_back(std::move(discFileItem));
//            break;
//        }
//    }
//
//    void Complete() override {
//        if (format == Format::RelativeDiscPath) { // remove root path
//            for (auto& dir : dirs) {
//                dir.discPath = dir.discPath.relative_path();
//            }
//            for (auto& file : files) {
//                file.discPath = file.discPath.relative_path();
//            }
//        }
//    }
//
//private:
//    uint64_t totalSize;
//    const Format format;
//    std::filesystem::path discRootPath;
//    std::vector<DiscFileItem> dirs;
//    std::vector<DiscFileItem> files;
//};
//
//
//
//
//void TestFilesCollection() {    
//    //std::filesystem::path tmpPath = L"C:\\TORRENT_TMP\\Hello\\";
//    std::filesystem::path tmpPath = L"\\TORRENT_TMP\\Hello";
//    auto rootPath = tmpPath.root_path();
//    auto relPath = tmpPath.relative_path();
//    auto filename = tmpPath.filename();
//
//
//    //auto xx = rootPath.re
//
//    std::vector<std::filesystem::path> fileList = {
//        L"D:\\black.png",
//        L"D:\\DiskDir_2",
//        L"C:\\TORRENT_TMP\\Hello",
//    };
//
//    auto filesCollection_implicit = FilesObserver::GetFilesCollection(fileList);
//
//    FilesCollection filesCollection_explicit;
//    FilesObserver::GetFilesCollection(fileList, filesCollection_explicit);
//
//    DiscFilesCollection discFilesCollection_explicit('H');
//    FilesObserver::GetFilesCollection(fileList, discFilesCollection_explicit);
//
//    return;
//}
//
//
//void main() {
//    //TestCopyDirectoryContentToAnotherDirectory();
//    //TestPathToFile();
//    TestVolumeInfo();
//    //TestFilesCollection();
//
//    return;
//}