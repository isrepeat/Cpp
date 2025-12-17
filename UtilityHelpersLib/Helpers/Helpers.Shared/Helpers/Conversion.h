#pragma once
#include "common.h"
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <span>

namespace HELPERS_NS {
	template<typename T>
	std::string HexToString(T hexValue) {
		std::stringstream ss;
		ss << "0x" << std::hex << std::setw(sizeof(hexValue) * 2) << std::setfill('0') << hexValue;
		return ss.str();
	}

    template<typename T>
    T HexFromStringA(const std::string& hexString) {
        T hexValue;
        std::stringstream ss;
        ss << std::hex << hexString;
        ss >> hexValue;
        return hexValue;
    }

	template<typename T>
	T HexFromStringW(const std::wstring& hexString) {
		T hexValue;
		std::wstringstream ss;
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

#if _HAS_CXX20
    // Convert some array's byte to hex
    std::span<char>::iterator HexByte(unsigned char byte, std::span<char>::iterator arrIt, const std::span<char>::iterator arrItEnd);
#endif

    std::string VectorBytesToHexString(const std::vector<uint8_t>& data);

    template <typename From, typename To>
    struct EnumConvertor {
        static To Convert(From enumVal) {
            static_assert(false, "not found implementation");
        }
    };
}