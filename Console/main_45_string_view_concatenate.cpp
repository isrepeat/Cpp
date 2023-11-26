//#include <string_view>
//#include <utility>
//#include <iostream>
//
//// https://stackoverflow.com/questions/38955940/how-to-concatenate-static-strings-at-compile-time
//
//namespace impl
//{
//    /// Base declaration of our constexpr string_view concatenation helper
//    template <typename T, const std::basic_string_view<T>&, typename, const std::basic_string_view<T>&, typename>
//    struct concat;
//
//    /// Specialisation to yield indices for each char in both provided string_views,
//    /// allows us flatten them into a single char array
//    template <typename T, const std::basic_string_view<T>& S1, std::size_t... I1, const std::basic_string_view<T>& S2, std::size_t... I2>
//    struct concat<T, S1, std::index_sequence<I1...>, S2, std::index_sequence<I2...>>
//    {
//        static constexpr const T value[]{ S1[I1]..., S2[I2]..., 0 };
//    };
//} // namespace impl
//
///// Base definition for compile time joining of strings
//template <typename T, const std::basic_string_view<T>&...> 
//struct join;
//
///// When no strings are given, provide an empty literal
//template <typename T>
//struct join<T>
//{
//    static constexpr std::basic_string_view<T> value = {};
//};
//
///// Base case for recursion where we reach a pair of strings, we concatenate
///// them to produce a new constexpr string
//template <typename T, const std::basic_string_view<T>& S1, const std::basic_string_view<T>& S2>
//struct join<T, S1, S2>
//{
//    static constexpr std::basic_string_view<T> value = 
//        impl::concat<T, S1, std::make_index_sequence<S1.size()>, S2, std::make_index_sequence<S2.size()>>::value;
//};
//
///// Main recursive definition for constexpr joining, pass the tail down to our
///// base case specialisation
//template <typename T, const std::basic_string_view<T>& S, const std::basic_string_view<T>&... Rest>
//struct join<T, S, Rest...>
//{
//    static constexpr std::basic_string_view<T> value = 
//        join<T, S, join<T, Rest...>::value>::value;
//};
//
///// Join constexpr string_views to produce another constexpr string_view
//template <typename T, const std::basic_string_view<T>&... Strs>
//static constexpr auto join_v = join<T, Strs...>::value;
//
//
//using Tchar = wchar_t;
//
//namespace str
//{
//    constexpr std::basic_string_view<Tchar> a = L"Hello ";
//    constexpr std::basic_string_view<Tchar> b = L"world ";
//    constexpr std::basic_string_view<Tchar> c = L"!";
//}
//
//
//
////constexpr void logger(constexpr std::basic_string_view<Tchar>& format) {
////    constexpr std::basic_string_view<Tchar> prefix = L"[] ";
////    constexpr std::basic_string_view<Tchar> joined = join_v<Tchar, prefix, format>;
////}
//
//int main() {
//
//    constexpr std::basic_string_view<Tchar> joined = join_v<Tchar, str::a, str::b, str::c>;
//    auto constexpr sz = joined.size();
//
//    std::wstring_view wstr = joined;
//    return 0;
//}