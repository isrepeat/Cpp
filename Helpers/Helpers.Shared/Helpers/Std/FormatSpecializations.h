#pragma once
#include "Helpers/common.h"

#include <filesystem>
#include <format>

#if !defined(__cpp_lib_format_path) // C++23 feature-test: built-in formatter for std::filesystem::path
//
// ░ std::filesystem::path
// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
//
template<class CharT>
struct std::formatter<std::filesystem::path, CharT> {
	constexpr auto parse(std::basic_format_parse_context<CharT>& ctx) {
		return this->innerFormatter.parse(ctx); // делегируем парсинг спецификаторов ({:>10.3}, и т.п.)
	}

	template<class FormatContext>
	auto format(const std::filesystem::path& path, FormatContext& ctx) const {
		if constexpr (std::is_same_v<CharT, wchar_t>) {
			const auto& s = path.native(); // wide на Windows
			return this->innerFormatter.format(std::basic_string_view<wchar_t>{ s.data(), s.size() }, ctx);
		}
		else {
			// narrow-вариант с гарантированной UTF-8:
			// .string() на Windows зависит от системной ANSI-кодировки (может исказить не-ASCII),
			// поэтому используем .u8string() (UTF-8) и интерпретируем байты как char.
			const auto u8 = path.u8string(); // std::u8string (UTF-8)
			const auto sv = std::string_view{
				reinterpret_cast<const char*>(u8.data()),
				u8.size()
			};
			return this->innerFormatter.format(sv, ctx);
		}
	}

private:
	std::formatter<std::basic_string_view<CharT>, CharT> innerFormatter;
};
#endif