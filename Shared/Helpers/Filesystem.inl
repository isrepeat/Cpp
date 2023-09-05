#pragma once
#include <filesystem>
#include <functional>
#include <fstream>
#include <cassert>
#include <vector>

namespace H {
    namespace FS {
        inline uintmax_t Filesize(const std::filesystem::path& filepath) { // TODO: remove it and use std::filesystem::file_size()
            std::ifstream fs(filepath, std::ios::binary);
            if (!fs.is_open())
                return 0;

            fs.seekg(0, std::ios::end);
            uintmax_t filesize = fs.tellg();
            fs.seekg(0, std::ios::beg);

            return filesize;
        }

        inline bool RemoveBytesFromStart(const std::filesystem::path& filepath, uintmax_t countRemovedBytes, std::function<void(std::ofstream&)> beginWriteHandler = nullptr) {
            std::vector<char> fileData;
            {
                std::ifstream inFile(filepath, std::ios::binary);
                if (!inFile.is_open()) {
                    assert(false && " --> can't open file");
                    return false;
                }

                inFile.seekg(0, std::ios::end);
                uintmax_t origFilesize = inFile.tellg();

                assert(countRemovedBytes < origFilesize);
                if (countRemovedBytes > origFilesize) {
                    countRemovedBytes = origFilesize;
                }

                uintmax_t newFilesize = origFilesize - countRemovedBytes;
                inFile.seekg(countRemovedBytes, std::ios::beg); // set stream pointer after removed bytes

                fileData.resize(newFilesize);
                inFile.read(fileData.data(), fileData.size()); // start read from stream pointer
            }

            std::ofstream outFile(filepath, std::ios::binary);
            if (beginWriteHandler) {
                beginWriteHandler(outFile);
            }
            outFile.write(fileData.data(), fileData.size());
        }
    }
}
