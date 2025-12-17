#pragma once
#include "Conversion.h"
#include <format>

namespace HELPERS_NS {
#if _HAS_CXX20
    std::span<char>::iterator HexByte(unsigned char byte, std::span<char>::iterator arrIt, const std::span<char>::iterator arrItEnd) {
        static const char* digits{ "0123456789ABCDEF" }; // NOTE: byte must be unsigned value

        if (arrIt < arrItEnd - 1) {
            *arrIt++ = digits[byte >> 4];
            *arrIt++ = digits[byte & 0x0f];
        }
        return arrIt;
    }
#endif

    std::string VectorBytesToHexString(const std::vector<uint8_t>& data) {
        std::stringstream ss;
        for (const auto& item : data) {
            ss << std::format("0x{:0>2X} ", item);
        }
        return ss.str();
    }
}