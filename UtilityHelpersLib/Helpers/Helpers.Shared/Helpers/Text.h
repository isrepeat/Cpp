#pragma once
#include "common.h"
#include "String.h"

#include <system_error>
#include <string_view>
#include <utility>
#include <format>
#include <vector>
#include <string>

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

namespace HELPERS_NS {
	namespace Text {
		// Универсальные ошибки конвертации как std::system_error
		inline std::system_error MakeWin32Error(
			char const* where
		) {
			const DWORD err = ::GetLastError();
			return std::system_error(
				static_cast<int>(err),
				std::system_category(),
				where
			);
		}

		//
		// ░ Узкие -> широкие (любая кодовая страница)
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		inline std::wstring MultiByteToWide(
			std::string_view bytes,
			UINT codePage,
			DWORD flags = MB_ERR_INVALID_CHARS
		) {
			if (bytes.empty()) {
				return std::wstring{};
			}

			// Может быть > INT_MAX на теории, проверим
			if (bytes.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
				throw std::length_error("MultiByteToWide: input too large");
			}

			int const srcLen = static_cast<int>(bytes.size());

			// 1) Узнаём нужную длину UTF-16 без завершающего нуля
			int const wlen = ::MultiByteToWideChar(
				codePage,
				flags,
				bytes.data(),
				srcLen,
				nullptr,
				0
			);
			if (wlen <= 0) {
				throw MakeWin32Error("MultiByteToWideChar(size)");
			}

			std::wstring wstr;
			wstr.resize(static_cast<size_t>(wlen));

			// 2) Конвертируем
			int const written = ::MultiByteToWideChar(
				codePage,
				flags,
				bytes.data(),
				srcLen,
				wstr.data(),
				wlen
			);
			if (written != wlen) {
				throw MakeWin32Error("MultiByteToWideChar(convert)");
			}

			return wstr;
		}

		//
		// ░ Широкие -> узкие (любая кодовая страница)
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		inline std::string WideToMultiByte(
			std::wstring_view wtext,
			UINT codePage,
			DWORD flags = WC_ERR_INVALID_CHARS
		) {
			if (wtext.empty()) {
				return std::string{};
			}

			if (wtext.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
				throw std::length_error("WideToMultiByte: input too large");
			}

			int const srcLen = static_cast<int>(wtext.size());

			int const blen = ::WideCharToMultiByte(
				codePage,
				flags,
				wtext.data(),
				srcLen,
				nullptr,
				0,
				nullptr,
				nullptr
			);
			if (blen <= 0) {
				throw MakeWin32Error("WideCharToMultiByte(size)");
			}

			std::string bytes;
			bytes.resize(static_cast<size_t>(blen));

			int const written = ::WideCharToMultiByte(
				codePage,
				flags,
				wtext.data(),
				srcLen,
				bytes.data(),
				blen,
				nullptr,
				nullptr
			);
			if (written != blen) {
				throw MakeWin32Error("WideCharToMultiByte(convert)");
			}

			return bytes;
		}

		//
		// ░ Удобные алиасы для UTF-8 <-> UTF-16
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		inline std::wstring Utf8ToUtf16(std::string_view bytes) {
			return MultiByteToWide(
				bytes,
				CP_UTF8,
				MB_ERR_INVALID_CHARS
			);
		}

		inline std::string Utf16ToUtf8(std::wstring_view wtext) {
			return WideToMultiByte(
				wtext,
				CP_UTF8,
				WC_ERR_INVALID_CHARS
			);
		}

		//
		// ░ Вспомогательные векторные варианты
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		inline std::wstring VecToWString(std::vector<wchar_t> const& vec) {
			return std::wstring{ vec.begin(), vec.end() };
		}

		inline std::string VecToString(std::vector<char> const& vec) {
			return std::string{ vec.begin(), vec.end() };
		}


		// Стандартные 16 ANSI-цветов для консоли (8 базовых + 8 ярких) + Default.
		enum class Color : unsigned char {
			Default = 0,

			// Базовые (30–37)
			Black,
			Red,
			Green,
			Yellow,
			Blue,
			Magenta,
			Cyan,
			White,

			// Яркие (90–97)
			BrightBlack,
			BrightRed,
			BrightGreen,
			BrightYellow,
			BrightBlue,
			BrightMagenta,
			BrightCyan,
			BrightWhite,

			Gray = BrightBlack,
		};

		//
		// ░ ConsoleColored
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		template<class TValue>
		class ConsoleColored final {
		public:
			explicit ConsoleColored(
				const TValue& value,
				const Color color
			)
				: value{ value }
				, color{ color } {
			}

			const TValue& Value() const {
				return this->value;
			}

			Color Color() const {
				return this->color;
			}

		private:
			const TValue& value;
			Text::Color color;
		};

		template<class TValue>
		ConsoleColored<TValue> Colored(
			const TValue& value,
			const Color color
		) {
			return ConsoleColored<TValue>{ value, color };
		}

		template<class TValue>
		constexpr ConsoleColored<TValue> operator|(
			const TValue& valueTValue,
			const Color   color
			) noexcept {
			return ConsoleColored<TValue>{ valueTValue, color };
		}

		// Вспомогательные ANSI-коды. Нешаблонные — можно унести в .cpp при желании.
		inline std::string_view AnsiPrefixNarrow(const Color color) {
			switch (color) {
			case Color::Black:          return "\x1b[30m";
			case Color::Red:            return "\x1b[31m";
			case Color::Green:          return "\x1b[32m";
			case Color::Yellow:         return "\x1b[33m";
			case Color::Blue:           return "\x1b[34m";
			case Color::Magenta:        return "\x1b[35m";
			case Color::Cyan:           return "\x1b[36m";
			case Color::White:          return "\x1b[37m";

			case Color::BrightBlack:    return "\x1b[90m";
			case Color::BrightRed:      return "\x1b[91m";
			case Color::BrightGreen:    return "\x1b[92m";
			case Color::BrightYellow:   return "\x1b[93m";
			case Color::BrightBlue:     return "\x1b[94m";
			case Color::BrightMagenta:  return "\x1b[95m";
			case Color::BrightCyan:     return "\x1b[96m";
			case Color::BrightWhite:    return "\x1b[97m";

			case Color::Default:        return "\x1b[39m";
			}
			return "\x1b[39m";
		}

		inline std::wstring_view AnsiPrefixWide(const Color color) {
			switch (color) {
			case Color::Black:          return L"\x1b[30m";
			case Color::Red:            return L"\x1b[31m";
			case Color::Green:          return L"\x1b[32m";
			case Color::Yellow:         return L"\x1b[33m";
			case Color::Blue:           return L"\x1b[34m";
			case Color::Magenta:        return L"\x1b[35m";
			case Color::Cyan:           return L"\x1b[36m";
			case Color::White:          return L"\x1b[37m";

			case Color::BrightBlack:    return L"\x1b[90m";
			case Color::BrightRed:      return L"\x1b[91m";
			case Color::BrightGreen:    return L"\x1b[92m";
			case Color::BrightYellow:   return L"\x1b[93m";
			case Color::BrightBlue:     return L"\x1b[94m";
			case Color::BrightMagenta:  return L"\x1b[95m";
			case Color::BrightCyan:     return L"\x1b[96m";
			case Color::BrightWhite:    return L"\x1b[97m";

			case Color::Default:        return L"\x1b[39m";
			}
			return L"\x1b[39m";
		}

		inline constexpr std::string_view AnsiResetNarrow = "\x1b[0m";
		inline constexpr std::wstring_view AnsiResetWide = L"\x1b[0m";
	}
} // namespace HELPERS_NS


//
// ░ std::formatter specializations
// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
//
// ░ ConsoleColored<TValue>
//
template<class TValue, class CharT>
struct std::formatter<HELPERS_NS::Text::ConsoleColored<TValue>, CharT> {
public:
	constexpr auto parse(std::basic_format_parse_context<CharT>& ctx) {
		return this->innerFormatterTValue.parse(ctx); // делегируем парсинг спецификаторов
	}

	template<class FormatContext>
	auto format(const HELPERS_NS::Text::ConsoleColored<TValue>& coloredValue, FormatContext& ctx) const {
		if constexpr (std::is_same_v<CharT, char>) {
			std::format_to(ctx.out(), "{}", HELPERS_NS::Text::AnsiPrefixNarrow(coloredValue.Color()));
			this->innerFormatterTValue.format(coloredValue.Value(), ctx);
			return std::format_to(ctx.out(), "{}", HELPERS_NS::Text::AnsiResetNarrow);
		}
		else {
			std::format_to(ctx.out(), L"{}",HELPERS_NS::Text::AnsiPrefixWide(coloredValue.Color()));
			this->innerFormatterTValue.format(coloredValue.Value(), ctx);
			return std::format_to(ctx.out(), L"{}", HELPERS_NS::Text::AnsiResetWide);
		}
	}

private:
	std::formatter<TValue, CharT> innerFormatterTValue;
};



#if COMPILE_FOR_CLR
#include <msclr/marshal.h>

namespace HELPERS_NS {
	namespace CLR {
		namespace Text {
			inline std::string ToUtf8(System::String^ s) {
				if (s == nullptr) {
					return {};
				}

				cli::array<System::Byte>^ bytes = System::Text::Encoding::UTF8->GetBytes(s);

				std::string out;
				out.resize(bytes->Length);

				// Копируем за один проход
				if (bytes->Length > 0) {
					cli::pin_ptr<System::Byte> p = &bytes[0];
					std::memcpy(out.data(), p, static_cast<size_t>(bytes->Length));
				}

				return out;
			}


			inline std::wstring ToWStr(System::String^ s) {
				if (s == nullptr) {
					return std::wstring{};
				}

				static_assert(sizeof(wchar_t) == 2, "wchar_t must be UTF-16 on Windows");

				// .NET String — UTF-16. Получаем прямой доступ к внутреннему буферу
				cli::pin_ptr<const wchar_t> pinned = ::PtrToStringChars(s);

				// Копируем содержимое в std::wstring
				return std::wstring{
					static_cast<const wchar_t*>(pinned),
					static_cast<std::size_t>(s->Length)
				};
			}


			// Требует: MSVC/Windows, где wchar_t == 2 байта (UTF-16)
			inline std::u16string ToU16(System::String^ s) {
				if (s == nullptr) {
					return {};
				}

				static_assert(sizeof(wchar_t) == 2, "wchar_t must be UTF-16 on Windows");

				// .NET String — всегда UTF-16; получаем pointer к внутреннему буферу
				cli::pin_ptr<const wchar_t> pinned = ::PtrToStringChars(s);

				// Без ручного «пошагового» копирования: допускается побайтовое копирование
				auto const* src16 = reinterpret_cast<const char16_t*>(static_cast<const wchar_t*>(pinned));

				std::u16string out;
				out.assign(src16, src16 + s->Length);

				return out;
			}


			inline System::String^ FromUtf8(std::string_view utf8) {
				if (utf8.empty()) {
					return gcnew System::String(L"");
				}

				const int len = static_cast<int>(utf8.size());
				cli::array<System::Byte>^ bytes = gcnew cli::array<System::Byte>(len);

				// Копируем в управляемый массив
				{
					cli::pin_ptr<System::Byte> dst = &bytes[0];
					std::memcpy(dst, utf8.data(), static_cast<std::size_t>(len));
				}

				return System::Text::Encoding::UTF8->GetString(bytes);
			}


			inline System::String^ FromWStr(std::wstring_view wstr) {
				if (wstr.empty()) {
					return gcnew System::String(L"");
				}

				// Прямой конструктор из wchar_t*
				return gcnew System::String(wstr.data(), 0, static_cast<int>(wstr.size()));
			}


			inline System::String^ FromU16(std::u16string_view u16) {
				if (u16.empty()) {
					return gcnew System::String(L"");
				}

				// Без перекодировки: .NET строка — UTF-16
				return gcnew System::String(
					reinterpret_cast<const wchar_t*>(u16.data()),
					0,
					static_cast<int>(u16.size())
				);
			}
		}
	}
}
#endif

#pragma pop_macro("max")
#pragma pop_macro("min")