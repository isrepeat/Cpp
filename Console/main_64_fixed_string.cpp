#include <Helpers/Logger.h>
#include <cstddef>
#include <string_view>

namespace H::Text {
	template <
		typename TChar,
		std::size_t N
	>
	struct FixedString {
	public:
		using char_t = TChar;

	public:
		// Конструируем из строкового литерала: N включает '\0'.
		constexpr FixedString(const TChar(&lit)[N]) {
			for (std::size_t i = 0; i < N; ++i) {
				this->data[i] = lit[i];
			}
		}

		// Длина без завершающего нуля.
		constexpr std::size_t size() const noexcept {
			return N - 1;
		}

		// Просмотр как string_view (без '\0').
		constexpr std::basic_string_view<TChar> view() const noexcept {
			return std::basic_string_view<TChar>{ this->data, this->size()};
		}

		// Индексация по символам без проверок.
		constexpr const TChar& operator[](std::size_t i) const noexcept {
			return this->data[i];
		}

	public:
		TChar data[N];
	};


	// C++20 CTAD-хелпер: make_ct("Hello")
	template <
		typename TChar,
		std::size_t N
	>
	constexpr auto make_fixed_string(const TChar(&lit)[N]) {
		return FixedString<TChar, N>{ lit };
	}


	// Конкатенация двух FixedString => новый FixedString (включая '\0').
	template <
		typename TChar,
		std::size_t L,
		std::size_t R
	>
	constexpr auto operator+(
		const FixedString<TChar, L>& left,
		const FixedString<TChar, R>& right
		) {
		FixedString<TChar, (L - 1) + R> out{ { 0 } };

		// Копируем left (без его '\0').
		for (std::size_t i = 0; i < L - 1; ++i) {
			out.data[i] = left[i];
		}

		// Копируем right (включая его '\0').
		for (std::size_t j = 0; j < R; ++j) {
			out.data[(L - 1) + j] = right.data[j];
		}

		return out;
	}

}

constexpr auto Prefix = H::Text::make_fixed_string("LOG/");
constexpr auto Suffix = H::Text::make_fixed_string("/Init");
constexpr auto FullTag = Prefix + H::Text::make_fixed_string("Editor") + Suffix;

// Можно использовать в constexpr-контексте:
static_assert(FullTag.size() == 15);
static_assert(FullTag.view() == std::string_view{ "LOG/Editor/Init" });


int main() {
	return 0;
}