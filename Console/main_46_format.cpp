//#include <format>
//#include <string>
//
//struct MyType {
//};
//
//
//template<>
//struct std::formatter<MyType, wchar_t> {
//    template <typename ParseContext>
//    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
//        return ctx.end();
//    }
//
//    template <typename FormatContext>
//    auto format(const MyType& input, FormatContext& ctx) -> decltype(ctx.out()) {
//        return std::format_to(ctx.out(), L"{}", std::wstring{L"Привет"});
//    }
//};
//
//int main() {
//	auto res = std::format(L"MyType = {}", MyType{});
//	return 0;
//}