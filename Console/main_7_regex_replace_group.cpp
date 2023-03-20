//#include <iostream>
//#include <iterator>
//#include <regex>
//#include <string>
//
//int main()
//{
//    //std::string text = "Quick brown fox";
//    //std::regex vowel_re("a|e|i|o|u");
//
//    //// write the results to an output iterator
//    //std::regex_replace(std::ostreambuf_iterator<char>(std::cout),
//    //    text.begin(), text.end(), vowel_re, "*");
//
//    // construct a string holding the results
//    //std::cout << '\n' << std::regex_replace(text, vowel_re, "[$&]") << '\n';
//
//
//
//    std::string text = "<translation ref='12345'>...</translation>";
//    std::regex vowel_re("(<translation).+(translation>)");
//    auto res = std::regex_replace(text, vowel_re, "$1>__xxx__</$2");
//
//    return 0;
//}