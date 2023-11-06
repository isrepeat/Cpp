#include <Windows.h>
#include <Helpers/Helpers.h>
#include <filesystem>
#include <iostream>
#include <string>


const std::wstring rootDir = H::ExePathW() + L"\\TestFilesystem\\";

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


void TestPathToFile() {
    std::wstring filename = L"D:\\WORK\\TEST\\AAA\\BBB\\foo.txt";
    std::wstring folder = std::filesystem::path(filename).parent_path();
    return;
}



struct FilesCollection {
    enum class Format {
        Default,
        RelativePath,
    };

    FilesCollection(std::vector<std::filesystem::path> filePaths, Format format = Format::Default) {
        for (auto& item : filePaths) {
            if (std::filesystem::is_regular_file(item)) {
                files.push_back(item);
                totalSize += std::filesystem::file_size(item);
            }
            else if (std::filesystem::is_directory(item)) {
                dirs.push_back(item);

                for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(item)) {
                    if (dirEntry.is_regular_file()) {
                        files.push_back(dirEntry.path());
                        totalSize += std::filesystem::file_size(dirEntry);
                    }
                    else if (dirEntry.is_directory()) {
                        dirs.push_back(dirEntry);
                    }
                }
            }
        }

        if (format == Format::RelativePath) { // remove root path
            for (auto& file : files) {
                file = file.relative_path() / file.filename();
            }
            for (auto& dir : dirs) {
                dir = dir.relative_path();
            }
        }
    };

    const std::vector<std::filesystem::path>& GetFiles() const {
        return files;
    }

    const std::vector<std::filesystem::path>& GetDirs() const {
        return dirs;
    }

    uint64_t GetSize() const {
        return totalSize;
    }

private:
    std::vector<std::filesystem::path> files;
    std::vector<std::filesystem::path> dirs;
    uint64_t totalSize = 0;
};


void TestFilesCollection() {    
    std::vector<std::filesystem::path> fileList = {
        L"D:\\DiskDir_2",
        L"D:\\black.png",
    };

    auto filesCollection_1 = FilesCollection(fileList);
    auto filesCollection_2 = FilesCollection(fileList, FilesCollection::Format::RelativePath);

    return;
}

void main() {
    //TestCopyDirectoryContentToAnotherDirectory();
    //TestPathToFile();
    TestFilesCollection();
    return;
}