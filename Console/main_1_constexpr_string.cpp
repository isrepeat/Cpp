//#include <iostream>
//#include <utility>
//#include <vector>
//#include <string>
//#include <array>
//#include <map>
//
//
//constexpr std::pair<int, int> map_items[] = {
//    { 6, 7 },
//    { 10, 12 },
//    { 300, 5000 },
//};
////constexpr auto map_size = sizeof map_items / sizeof map_items[0];
//
//constexpr int arr[] = {
//    11,
//    22,
//    33,
//};
//
//template <std::size_t N>
//constexpr int ArraySize(int arr[N]) {
//    return N;
//}
//
//
////constexpr int findValue(const std::pair<int, int> mapItems[3], int key) {
//template <typename T, typename U, std::size_t N>
//constexpr U findValue(const std::pair<T, U>(&mapItems)[N], T key) {
//    for (int i = 0; i < N; i++) {
//        if (mapItems[i].first == key) {
//            return mapItems[i].second;
//        }
//    }
//    throw "Key not present";
//};
//
////static constexpr int findValue(const std::pair<int, int> mapItems[3], int key) {
////
////    for (int i = 0; i < 3; i++) {
////        std::pair<int, int> item = mapItems[i];
////        if (item.first == key) {
////            return item.second;
////        }
////    }
////
////    throw "Key not present";
////};
//
//
////static constexpr int findKey(int value, int range = map_size) {
////    return
////        (range == 0) ? throw "Value not present" :
////        (map_items[range - 1].second == value) ? map_items[range - 1].first :
////        findKey(value, range - 1);
////};
//
//
////std::array<std::pair<char, char>>
////constexpr std::pair<char, char> toUpperCaseMap {'a', 'A'};
//
//
//constexpr bool strings_equal(const char* a, const char* b) {
//    return std::string_view(a) == b;
//}
//
//
//constexpr char someString[] = "Hell";
//
//
////template <std::size_t N>
//constexpr const char(&modifiedString(const char(&string)[5])) [5] {
//    //string[0] = 'W';
//    //return string;
//
//    auto nonconstString = const_cast<char(&)[5]>(string);
//    nonconstString[0] = 'W';
//    return string;
//}
//
//////template <std::size_t N>
////const char* modifiedString(const char* string) {
////    //auto nonconstString = const_cast<char(&)[N]>(string);
////    //nonconstString[0] = 'W';
////    //return nonconstString;
////    return "aa";
////}
//
////constexpr bool isWell(char* str) {
////    
////}
//
////const char* testConstString = modifiedString(someString);
//
//
////template <auto N>
////constexpr auto replace_sub_str(char const (&src)[N]) {
////    std::array<char, N> res = {};
////    // do whatever string manipulation you want in res.
////    return res;
////}
////
////
////auto constexpr str = replace_sub_str("hello {}, {}");
//
//constexpr std::string_view sv1 = "WellXX";
//constexpr std::string_view sv2 = sv1.substr(0, 4); // view on "ab"
//
//constexpr bool isWell(std::string_view sv) {
//    return sv == "Well";
//}
//
//
//constexpr std::string_view FirstLetterToUpperCase(std::string_view word) {
//    if (word.size() == 0) {
//        throw;
//    }
//    
//    std::string_view restString = word.substr(1);
//    return "D" + restString;
//}
//
//
//int main() {
//    //static_assert(!strings_equal(modifiedString("aaa"), "Well"), "strings are NOT equal");
//    //static_assert(strings_equal(sv2.data(), "Well"), "strings are equal");
//    static_assert(sv2 == "Well", "strings are equal");
//    static_assert(FirstLetterToUpperCase("dog") == "Dog", "strings are equal");
//
//    //constexpr std::string_view res = FirstLetterToUpperCase("dog");
//
//    auto resultString = modifiedString(someString);
//
//    return 0;
//}
