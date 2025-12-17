#pragma once
#include "common.h"
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <string>
#include <format>
#include <array>

namespace HELPERS_NS {
	class Guid {
	public:
		enum Format {
			Uppercase,
			Lowercase,
			Default = Uppercase
		};

		static Guid NewGuid();

		Guid();
		explicit Guid(const std::string& guidStr);
		static Guid Parse(const std::string& guidStr);

		// Возвращает неизменяемую ссылку на массив байт (безопасный доступ)
		const std::array<uint8_t, 16>& GetBytesArray() const;

		// Возвращает указатель на начало массива (например, для reinterpret_cast)
		const uint8_t* data() const;

		template<typename CharT>
		std::basic_string<CharT> ToBasicString(Format format) const {
			std::basic_ostringstream<CharT> oss;
			oss << std::hex;

			if (format == Format::Uppercase) {
				oss << std::uppercase;
			}
			else if (format == Format::Lowercase) {
				oss << std::nouppercase;
			}

			oss << std::setfill(static_cast<CharT>('0'));
			oss << static_cast<CharT>('{');

			for (int i = 0; i < 16; ++i) {
				oss << std::setw(2) << static_cast<int>(this->bytesArray[i]);
				if (i == 3 || i == 5 || i == 7 || i == 9) {
					oss << static_cast<CharT>('-');
				}
			}

			oss << static_cast<CharT>('}');
			return oss.str();
		}

		std::string ToString(Format format = Format::Default) const;
		std::wstring ToWString(Format format = Format::Default) const;

		bool operator==(const Guid& other) const;
		bool operator!=(const Guid& other) const;
		bool operator<(const Guid& other) const;
		bool operator>(const Guid& other) const;
		bool operator<=(const Guid& other) const;
		bool operator>=(const Guid& other) const;

	private:
		std::array<uint8_t, 16> bytesArray;
	};
}


//
// ░ Std specializations
// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
//
// ░ std::hash
//
template<>
struct std::hash<HELPERS_NS::Guid> {
	std::size_t operator()(const HELPERS_NS::Guid& guid) const noexcept {
		const auto& bytesArray = guid.GetBytesArray();
		const uint64_t* pData = reinterpret_cast<const uint64_t*>(bytesArray.data());

		// Простой способ: XOR двух 64-битных слов
		return std::hash<uint64_t>{}(pData[0]) ^ std::hash<uint64_t>{}(pData[1]);
	}
};


//
// ░ std::formatter
//
template<>
struct std::formatter<H::Guid, char> {
	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const HELPERS_NS::Guid& guid, std::format_context& ctx) const {
		return std::format_to(ctx.out(), "{}", guid.ToString());
	}
};

template<>
struct std::formatter<HELPERS_NS::Guid, wchar_t> {
	constexpr auto parse(std::wformat_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const H::Guid& guid, std::wformat_context& ctx) const {
		return std::format_to(ctx.out(), L"{}", guid.ToWString());
	}
};