#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

namespace H {
	template<typename T>
	std::string HexToString(T hexValue) {
		std::stringstream ss;
		ss << "0x" << std::hex << std::setw(sizeof(hexValue) * 2) << std::setfill('0') << hexValue;
		return ss.str();
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
	T HexFromStringA(const std::string& hexString) {
		T hexValue;
		std::stringstream ss;
		ss << std::hex << hexString;
		ss >> hexValue;
		return hexValue;
	}
	}