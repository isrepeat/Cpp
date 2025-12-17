#include "Guid.h"
#include <random>

namespace HELPERS_NS {
	Guid Guid::NewGuid() {
		Guid g;
		std::random_device rd;
		std::mt19937 gen(rd());

		for (auto& byte : g.bytesArray) {
			byte = static_cast<uint8_t>(gen() & 0xFF);
		}

		// Версия 4 (random) + вариант (RFC4122)
		g.bytesArray[6] = (g.bytesArray[6] & 0x0F) | 0x40; // version 4
		g.bytesArray[8] = (g.bytesArray[8] & 0x3F) | 0x80; // variant

		return g;
	}

	Guid::Guid() {
		this->bytesArray.fill(0);
	}

	Guid::Guid(const std::string& guidStr) {
		*this = Guid::Parse(guidStr);
	}

	Guid Guid::Parse(const std::string& guidStr) {
		std::string s = guidStr;

		// Удалим скобки, если есть
		if (!s.empty() && s.front() == '{' && s.back() == '}') {
			s = s.substr(1, s.size() - 2);
		}

		// Ожидаемый формат: 8-4-4-4-12 = 36 символов с дефисами
		if (s.size() != 36 ||
			s[8] != '-' || s[13] != '-' || s[18] != '-' || s[23] != '-') {
			throw std::invalid_argument("Invalid GUID format: " + guidStr);
		}

		std::array<uint8_t, 16> bytes;
		const char* hex = s.c_str();
		const int idx[] = {
			0, 2, 4, 6, 9, 11, 14, 16, 19, 21, 24, 26, 28, 30, 32, 34
		};

		for (int i = 0; i < 16; ++i) {
			std::string byteStr{ hex[idx[i]], hex[idx[i] + 1] };
			bytes[i] = static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16));
		}

		Guid g;
		g.bytesArray = bytes;
		return g;
	}


	const std::array<uint8_t, 16>& Guid::GetBytesArray() const {
		return this->bytesArray;
	}

	const uint8_t* Guid::data() const {
		return this->bytesArray.data();
	}

	std::string Guid::ToString(Format format) const {
		return this->ToBasicString<char>(format);
	}

	std::wstring Guid::ToWString(Format format) const {
		return this->ToBasicString<wchar_t>(format);
	}


	bool Guid::operator==(const Guid& other) const {
		return this->bytesArray == other.bytesArray;
	}

	bool Guid::operator!=(const Guid& other) const {
		return !(*this == other);
	}

	bool Guid::operator<(const Guid& other) const {
		return this->bytesArray < other.bytesArray;
	}

	bool Guid::operator>(const Guid& other) const {
		return other < *this;
	}

	bool Guid::operator<=(const Guid& other) const {
		return !(other < *this);
	}

	bool Guid::operator>=(const Guid& other) const {
		return !(*this < other);
	}
}