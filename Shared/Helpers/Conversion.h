#pragma once
#include <span>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

namespace H {
	template<typename T>
	T HexFromString(const std::string& hexString) {
		T hexValue;
		std::stringstream ss;
		ss << std::hex << hexString;
		ss >> hexValue;
		return hexValue;
	}

    template<typename T>
    std::vector<uint8_t> NumToBytes(T src) {
        int size = sizeof T;
        if (size == 1)
            return { (uint8_t)src };

        std::vector<uint8_t> bytes(size);
        for (int i = 0; i < size; i++) {
            bytes[i] = uint8_t(src >> i * 8) & 0xFF;
        }
        return bytes;
    }


    template<typename T>
    T BytesToNum(const std::vector<uint8_t>& bytes) {
        int size = sizeof T;
        if (bytes.size() == 0 || bytes.size() < size)
            return 0;

        if (size == 1)
            return bytes[0];

        T num = 0;
        for (int i = 0; i < size; i++) {
            num |= T(bytes[i]) << (i * 8);
        }
        return num;
    }

    // Convert some array's byte to hex
    std::span<char>::iterator HexByte(unsigned char byte, std::span<char>::iterator arrIt, const std::span<char>::iterator arrItEnd);

    std::string VectorBytesToHexString(const std::vector<uint8_t>& data);
}