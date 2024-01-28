#include <cstddef>
#include <string>
#include <vector>
#include <array>
#include <string_view>
#include <initializer_list>
#include <iterator>
#include <limits>

// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0732r0.pdf


namespace std {
	template <typename CharT, std::size_t N>
	struct basic_fixed_string {
		constexpr basic_fixed_string(const CharT(&foo)[N]) {
			std::copy(foo, foo + N, m_data);
		}
		//auto operator<=>(const basic_fixed_string&, const basic_fixed_string&) = default;
		CharT m_data[N];

		static constexpr const CharT value[] = "asdasdasd";
	};
	
	//template <typename CharT, std::size_t N>
	//basic_fixed_string(const CharT(&str)[N])->basic_fixed_string<CharT, N - 1>;
	
	//template <std::size_t N>
	//using fixed_string = basic_fixed_string<char, N>;


	template <std::size_t N, std::basic_fixed_string<char, N> Str>
	struct A {};

	//template <std::basic_fixed_string Str>
	//struct A {};
}

namespace v1 {
	template <typename CharT, std::size_t N, typename>
	struct basic_fixed_string;

	template <typename CharT, std::size_t N, std::size_t... I>
	struct basic_fixed_string<CharT, N, std::index_sequence<I...>> {
		constexpr basic_fixed_string(const CharT(&foo)[N])
			//: value{"asdasdas"}
		{
			std::copy(foo, foo + N, m_data);
		}
		
		CharT m_data[N];
		
		static constexpr const CharT value[] = "xxxxxx";
		//static constexpr const CharT value[];
	};

	//template <typename CharT, std::size_t N, std::size_t... I>
	//basic_fixed_string(const CharT(&str)[N])->basic_fixed_string<CharT, N, std::make_index_sequence<N>>;

	template <std::size_t N, v1::basic_fixed_string<char, N, std::make_index_sequence<N>> Str>
	struct A {
		//using str_t = v1::basic_fixed_string<char, N, std::make_index_sequence<N>>;
	};
}

namespace v3 {
	template <std::size_t N, typename>
	struct basic_fixed_string;

	//template <std::size_t N, char L>
	////struct basic_fixed_string<N, std::array<Temp, L>> {
	//struct basic_fixed_string<N, Temp<L>> {

	
	//template <std::size_t N, std::array<int, N>>
	template <std::size_t N, std::size_t... I>
	struct basic_fixed_string<N, std::index_sequence<I...>> {

	//template <typename CharT, std::size_t N>
	//struct basic_fixed_string {
		static constexpr const char value[] = "xxxxxx";
		static constexpr const char value2[] = { value[I]...,'\0' };

		constexpr basic_fixed_string(const char(&foo)[N])
		//	: value{"asdasdas"}
		{
			//static constexpr const char string[] = { foo[I]...,'\0' };
			std::copy(foo, foo + N, m_data);
		}

		char m_data[N];
	};

	template <std::size_t N, std::size_t... I>
	basic_fixed_string(const char(&str)[N])->basic_fixed_string<N, std::index_sequence<I...>>;

	//template <std::size_t N, v3::basic_fixed_string<char, N, std::array<int, N>> Str >
	//struct A {
	//};

	template <v3::basic_fixed_string Str>
	struct A {
	};
}


namespace v4 {
	template <std::size_t N, typename>
	struct basic_fixed_string;

	template <std::size_t N, std::size_t... I>
	struct basic_fixed_string<N, std::index_sequence<I...>> {

		//template <typename CharT, std::size_t N>
		//struct basic_fixed_string {

		char m_data[N];
		//static constexpr const char value2[] = { m_data[I]...,'\0' };
		
		static constexpr const char value[] = "xxxxxx";
		static constexpr const char value2[] = { value[I]...,'\0' }; // not work

		constexpr basic_fixed_string(const char(&foo)[N])
			//	: value{"asdasdas"}
		{
			//static constexpr const char string[] = { foo[I]...,'\0' };
			std::copy(foo, foo + N, m_data);
		}

		
	};

	template <std::size_t N, std::size_t... I>
	basic_fixed_string(const char(&str)[N])->basic_fixed_string<N, std::index_sequence<I...>>;

	template <v4::basic_fixed_string Str>
	struct A {
		using str_t = decltype(Str);
	};

	template <const char* Str>
	struct B {
		using str_t = decltype(Str);
	};
}








const char* strPtr = nullptr;

void bar() {
	constexpr auto res = v3::basic_fixed_string("hello");
	strPtr;
}

void foo() {
	constexpr auto res = v3::basic_fixed_string("hello");
	strPtr = res.value;
}

int main() {
	//foo();
	//bar();
	
	//constexpr v3::basic_fixed_string<char, 6, std::array<int, 6>> xxx("hello");

	//v1::A<6, "hello">;// ::str_t::value;
	//v3::A<6, "hello">;
	//v3::A<6, v3::basic_fixed_string<6, std::array<Temp, 2>>("hello")>;
	//v3::A<6, v3::basic_fixed_string<6, Temp<2>>("hello")>;
	//static_assert(v4::A<v4::basic_fixed_string("hello")>::str_t::value2[0] == 'x', "");
	const char* strPtr = v4::A<v4::basic_fixed_string("hello")>::str_t::value;
	

	return 0;
}