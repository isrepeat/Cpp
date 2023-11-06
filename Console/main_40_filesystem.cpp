#include <Windows.h>
#include <Helpers/Helpers.h>
#include <filesystem>
#include <iostream>
#include <string>


const std::wstring rootDir = H::ExePathW() + L"\\TestFilesystem\\";

/* ---------------------------------------------- */
/*   TestCopyDirectoryContentToAnotherDirectory   */
/* ---------------------------------------------- */
void TestCopyDirectoryContentToAnotherDirectory() {
    std::wstring fromDir = rootDir + L"Logs";
    std::wstring toDir = rootDir +  L"CrashReport";

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




/* ----------------------- */
/*   TestFilesCollection   */
/* ----------------------- */
#define define_has_member(member_name)                                         \
    template <typename T>                                                      \
    class has_member_##member_name                                             \
    {                                                                          \
        typedef char yes_type;                                                 \
        typedef long no_type;                                                  \
        template <typename U> static yes_type test(decltype(&U::member_name)); \
        template <typename U> static no_type  test(...);                       \
    public:                                                                    \
        static constexpr bool value = sizeof(test<T>(0)) == sizeof(yes_type);  \
    }

#define has_member(class_, member_name)  has_member_##member_name<class_>::value

struct IFilesCollection {
    //enum class Type {
    //    RegularFile,
    //    Directory,
    //};

    virtual ~IFilesCollection() = default;

protected:
    friend class FilesObserver;
    virtual void Initialize() = 0;
    virtual void HandlePathItem(std::filesystem::path item) = 0;
    virtual void Complete() = 0;
};


struct FilesCollection : public IFilesCollection {
    FilesCollection() = default;

    void ReplaceRootPath(std::wstring newRootPath) {
        for (auto& dir : dirs) {
            dir = newRootPath / dir.relative_path();
        }
        for (auto& file : files) {
            file = newRootPath / file.relative_path() / file.filename();
        }
    }

    const std::vector<std::filesystem::path>& GetDirs() const {
        return dirs;
    }

    const std::vector<std::filesystem::path>& GetFiles() const {
        return files;
    }

    const uint64_t GetSize() const {
        return totalSize;
    }

private:
    void Initialize() override {
        dirs.clear();
        files.clear();
        totalSize = 0;
    }

    void HandlePathItem(std::filesystem::path item) override {
        if (std::filesystem::is_regular_file(item)) {
            files.push_back(item);
            totalSize += std::filesystem::file_size(item);
        }
        else if (std::filesystem::is_directory(item)) {
            dirs.push_back(item);
        }
    }

    void Complete() override {
    }

private:
    std::vector<std::filesystem::path> dirs;
    std::vector<std::filesystem::path> files;
    uint64_t totalSize = 0;
};



struct DiscItem {
    std::filesystem::path localPath;
    std::filesystem::path discPath;
};

struct DiscFilesCollection : public IFilesCollection {
    DiscFilesCollection(wchar_t discLabel) 
        : diskRootPath{ discLabel + std::wstring(L":\\") }
    {
    }

    const std::vector<DiscItem>& GetDirs() const {
        return dirs;
    }

    const std::vector<DiscItem>& GetFiles() const {
        return files;
    }

    const uint64_t GetSize() const {
        return totalSize;
    }

private:
    void Initialize() override {
        dirs.clear();
        files.clear();
        totalSize = 0;
    }

    void HandlePathItem(std::filesystem::path item) override {
        if (std::filesystem::is_regular_file(item)) {
            files.push_back(DiscItem{ item, diskRootPath / item.relative_path() });
            totalSize += std::filesystem::file_size(item);
        }
        else if (std::filesystem::is_directory(item)) {
            dirs.push_back(DiscItem{ item, diskRootPath / item.relative_path() });
        }
    }

    void Complete() override {
    }

private:
    std::wstring diskRootPath;
    std::vector<DiscItem> dirs;
    std::vector<DiscItem> files;
    uint64_t totalSize = 0;
};


class FilesObserver {
    FilesObserver() = delete;

public:
    template <typename TCollection>
    static void GetFilesCollection(const std::vector<std::filesystem::path>& filePaths, TCollection& filesCollection) {
        IFilesCollection& filesCollectionInterface = filesCollection; // TODO: use type traits on base class to validate static_cast at compile time

        filesCollectionInterface.Initialize();

        for (auto& item : filePaths) {
            if (std::filesystem::is_regular_file(item)) {
                filesCollectionInterface.HandlePathItem(item);
            }
            else if (std::filesystem::is_directory(item)) {
                filesCollectionInterface.HandlePathItem(item);

                for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(item)) {
                    filesCollectionInterface.HandlePathItem(dirEntry.path());
                }
            }
        }
        filesCollectionInterface.Complete();
    };

    template <typename TCollection = FilesCollection>
    static TCollection GetFilesCollection(const std::vector<std::filesystem::path>& filePaths) {
        TCollection filesCollection; // TCollection must have default Ctor
        GetFilesCollection<TCollection>(filePaths, filesCollection);
        return filesCollection;
    };
};


void TestFilesCollection() {    
    std::vector<std::filesystem::path> fileList = {
        L"D:\\black.png",
        L"D:\\DiskDir_2",
        L"C:\\TORRENT_TMP\\Hello",
    };

    auto rel_path = fileList[0].relative_path();
    auto root_path = fileList[0].root_path();

    auto filesCollection_implicit = FilesObserver::GetFilesCollection(fileList);

    FilesCollection filesCollection_explicit;
    FilesObserver::GetFilesCollection(fileList, filesCollection_explicit);

    //auto discFilesCollection_explicit = FilesObserver::GetFilesCollection<DiscFilesCollection>(fileList);

    DiscFilesCollection discFilesCollection_explicit('H');
    FilesObserver::GetFilesCollection(fileList, discFilesCollection_explicit);

    return;
}

void main() {
    //TestCopyDirectoryContentToAnotherDirectory();
    //TestPathToFile();
    TestFilesCollection();
    return;
}