//#include <iostream>
//
//template <typename T>
//void Foo(T str) {
//	if constexpr (std::is_constructible_v<std::string, T>) {
//		static_assert(false, "This is std::string");
//	}
//	else if constexpr (std::is_constructible_v<std::wstring, T>) {
//		static_assert(false, "This is std::wstring");
//	}
//	else {
//		static_assert(false, "Bad arg");
//	}
//}
//
//
//int main() {
//
//	Foo(L"");
//
//	return 0;
//}
