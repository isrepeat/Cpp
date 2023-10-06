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

void main() {
    //TestCopyDirectoryContentToAnotherDirectory();
    TestPathToFile();
    return;
}