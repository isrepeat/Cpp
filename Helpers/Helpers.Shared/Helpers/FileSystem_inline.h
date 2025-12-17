#pragma once
#include "common.h"
#if COMPILE_FOR_DESKTOP || COMPILE_FOR_CX
#include <filesystem>
#include <functional>
#include <fstream>
#include <cassert>
#include <vector>

namespace HELPERS_NS {
    namespace FS {
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

            return true;
        }

        inline void PrependToFile(const std::filesystem::path& filePath, const char* data, size_t dataSize) {
            std::ifstream inFile(filePath, std::ios::binary);
            if (!inFile.is_open()) {
                return;
            }

            inFile.seekg(0, std::ios::end);
            uintmax_t oldDataSize = inFile.tellg();
            inFile.seekg(0, std::ios::beg);
            std::vector<char> oldData;
            oldData.resize(oldDataSize);
            inFile.read(oldData.data(), oldData.size());
            inFile.close();

            std::ofstream outFile(filePath, std::ios::binary);
            if (!outFile.is_open()) {
                return;
            }

            outFile.write(data, dataSize);
            outFile.write(oldData.data(), oldData.size());
            outFile.close();
        }
    }
}
#endif