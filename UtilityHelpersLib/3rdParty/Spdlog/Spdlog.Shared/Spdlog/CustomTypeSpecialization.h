#pragma once // Include this file after spdlog headers
#include "MagicEnum/MagicEnum.h"
#include "Helpers/Flags.h"
#include <filesystem>
#include <string>
#include <ranges>

namespace details {
	inline std::string WStrToStr(const std::wstring& wstr, int codePage = CP_UTF8) {
		if (wstr.size() == 0)
			return std::string{};

		int sz = WideCharToMultiByte(codePage, 0, wstr.c_str(), -1, 0, 0, 0, 0);
		std::string res(sz, 0);
		WideCharToMultiByte(codePage, 0, wstr.c_str(), -1, &res[0], sz, 0, 0);
		return res.c_str(); // To delete '\0' use .c_str()
	}

	inline std::wstring StrToWStr(const std::string& str, int codePage = CP_UTF8) {
		if (str.size() == 0)
			return std::wstring{};

		int sz = MultiByteToWideChar(codePage, 0, str.c_str(), -1, 0, 0);
		std::wstring res(sz, 0);
		MultiByteToWideChar(codePage, 0, str.c_str(), -1, &res[0], sz);
		return res.c_str();
	}
}

//
// std::filesystem::path
//
// WARNING: Log dirrectly path.string() may cause some problem (for example for shell item path)
//          therefore convert it to UTF8.
template<>
struct fmt::formatter<std::filesystem::path, char> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }
    auto format(const std::filesystem::path& path, format_context& ctx) -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", details::WStrToStr(path.wstring(), CP_UTF8));
    }
};

template<>
struct fmt::formatter<std::filesystem::path, wchar_t> {
    constexpr auto parse(wformat_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }
    auto format(const std::filesystem::path& path, wformat_context& ctx) -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), L"{}", details::StrToWStr(path.string(), CP_UTF8));
    }
};

//
// H::Flags
//
template<typename EnumT>
struct fmt::formatter<HELPERS_NS::Flags<EnumT>, char> {
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.end();
	}
	auto format(const HELPERS_NS::Flags<EnumT>& flags, format_context& ctx) -> decltype(ctx.out()) {
#if LOG_FORMAT_FLAGS_ENABLE_ENUM_EXPANSION // TODO: enable expansion flags to enum strings through logger options
		std::vector<std::string> flagNames;
		flags.ProcessAllFlags([&](EnumT enumType) {
			std::string_view strView = MagicEnum::ToString(enumType);
			flagNames.push_back({ strView.begin(), strView.end() });
			return false;
			});

		std::string joinedFlagsString;
		if (!flagNames.empty()) {
			joinedFlagsString += flagNames[0];

			for (auto& flagName : std::ranges::drop_view{ flagNames, 1 }) {
				joinedFlagsString += " | ";
				joinedFlagsString += flagName;
			}
		}
		return fmt::format_to(ctx.out(), "0b{:08b} [{}]" // TODO: also manage width through macros or logger option
			, static_cast<HELPERS_NS::Flags<EnumT>::UnderlyingType>(flags)
			, joinedFlagsString
		);
#else
		return fmt::format_to(ctx.out(), "0b{:08b}"
			, static_cast<HELPERS_NS::Flags<EnumT>::UnderlyingType>(flags)
		);
#endif
	}
};

template<typename EnumT>
struct fmt::formatter<HELPERS_NS::Flags<EnumT>, wchar_t> {
	constexpr auto parse(wformat_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.end();
	}
	auto format(const HELPERS_NS::Flags<EnumT>& flags, wformat_context& ctx) -> decltype(ctx.out()) {
#if LOG_FORMAT_FLAGS_ENABLE_ENUM_EXPANSION
		std::vector<std::wstring> flagNames;
		flags.ProcessAllFlags([&](EnumT enumType) {
			std::string_view strView = MagicEnum::ToString(enumType);
			flagNames.push_back({ strView.begin(), strView.end() });
			return false;
			});

		std::wstring joinedFlagsString;
		if (!flagNames.empty()) {
			joinedFlagsString += flagNames[0];

			for (auto& flagName : std::ranges::drop_view{ flagNames, 1 }) {
				joinedFlagsString += L" | ";
				joinedFlagsString += flagName;
			}
		}

		return fmt::format_to(ctx.out(), L"0b{:08b} [{}]"
			, static_cast<HELPERS_NS::Flags<EnumT>::UnderlyingType>(flags)
			, joinedFlagsString
		);
#else
		return fmt::format_to(ctx.out(), L"0b{:08b}"
			, static_cast<HELPERS_NS::Flags<EnumT>::UnderlyingType>(flags)
		);
#endif
	}
};


#ifdef QT_CORE_LIB
#include <QString>

template<>
struct fmt::formatter<QString, char> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }
    auto format(const QString& qtString, format_context& ctx) -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", qtString.toStdString());
    }
};

template<>
struct fmt::formatter<QString, wchar_t> {
    constexpr auto parse(wformat_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }
    auto format(const QString& qtString, wformat_context& ctx) -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), L"{}", qtString.toStdWString());
    }
};
#endif