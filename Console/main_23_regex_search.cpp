//#include <regex>
//#include <string>
//#include <cassert>
//#include <iostream>
//
////std::vector<std::vector<std::wstring>> GetRegexCapturedGroups(const std::wstring& text, const std::wregex& rx) {
////    std::vector<std::vector<std::wstring>> captured_groups;
////    std::vector<std::wstring> captured_subgroups;
////    const std::wsregex_token_iterator end_i;
////    for (std::wsregex_token_iterator i(text.cbegin(), text.cend(), rx); i != end_i; ++i) {
////        captured_subgroups.clear();
////        std::wstring group = *i;
////        std::wsmatch res;
////        if (regex_search(group, res, rx)) {
////            for (unsigned i = 0; i < res.size(); i++)
////                captured_subgroups.push_back(res[i]);
////
////            if (captured_subgroups.size() > 0)
////                captured_groups.push_back(captured_subgroups);
////        }
////    }
////    return captured_groups;
////}
//
//struct RegexMatchResult {
//    RegexMatchResult(const std::wsmatch& match) {
//        for (auto& res : match) {
//            matches.push_back(res);
//        }
//
//        preffix = match.prefix();
//        suffix = match.suffix();
//    }
//
//    std::vector<std::wstring> matches;
//    std::wstring preffix;
//    std::wstring suffix;
//};
//
//std::vector<RegexMatchResult> GetRegexMatches(const std::wstring& text, const std::wregex& rx) {
//    std::vector<RegexMatchResult> matches;
//    const std::wsregex_token_iterator end_i;
//    for (std::wsregex_token_iterator i(text.cbegin(), text.cend(), rx); i != end_i; ++i) {
//        std::wstring matchString = *i;
//        std::wsmatch matchResult;
//        if (std::regex_search(matchString, matchResult, rx)) {
//            matches.push_back(matchResult);
//        }
//    }
//    return matches;
//}
//
//bool FindInsideTagWithRegex(const std::wstring& text, const std::wstring& tag, const std::wregex& innerRx) {
//    const std::wregex rx(L"([^<]*)<" + tag + L"[^>]*>(.+?)<[/]" + tag + L">([^<]*)");
//
//    auto matches = GetRegexMatches(text, rx);
//    for (auto& match : matches) {
//        std::wsmatch matchResult;
//        if (std::regex_search(match.matches[2], matchResult, innerRx)) {
//            return true;
//        }
//    }
//    return false;
//}
//
//int main() {
//    //const std::string input = "ABC:1->   PQR:2;;;   XYZ:3<<<";
//    //const std::regex r("(\w+):(\w+);");
//    //std::smatch m;
//    //if (std::regex_search(input, m, r)) {
//    //    assert(m.size() == 3);
//    //    assert(m[0].str() == "PQR:2;");                // Entire match
//    //    assert(m[1].str() == "PQR");                   // Substring that matches 1st group
//    //    assert(m[2].str() == "2");                     // Substring that matches 2nd group
//    //    assert(m.prefix().str() == "ABC:1->   ");      // All before 1st character match
//    //    assert(m.suffix().str() == ";;   XYZ:3<<<");   // All after last character match
//
//    //     for (std::string &&str : m) { // Alternatively. You can also do
//    //         std::cout << str << std::endl;
//    //     }
//    //}
//
//
//    //const std::wstring input = L"Hello <a href=\"https://google.com/\">Click <p>Me</p> One</a> world <a href=\"https://google.com/\">Click two</a> the end";
//    //const std::string input = "Hello <a href=\"https://google.com/\">Click One</a> world <a href=\"https://google.com/\">Click two</a> the end";
//    const std::wregex rx(L"([^<]*)<a[^>]*>(.+?)<[/]a>([^<]*)");
//    //const std::wregex rx(L"([^<]*)<a[^>]*>(.+?)<[/]a>");
//    //const std::wregex rx(L"<a [^>]+>(.+?)<[/]a>");
//    
//    const std::wstring input = L"Hello <a>Click One</a> world <a>Click two</a> and <a>Click three</a> the end";
//    //const std::regex rx("<a>([^<]+)</a>");
//
//    bool found = FindInsideTagWithRegex(L"<name>Information</name>", L"name", std::wregex(L"^Information$"));
//
//    //auto res = GetRegexCapturedGroups(input, rx);
//    auto matches = GetRegexMatches(input, rx);
//
//    for (auto match : matches) {
//    }
//
//    //std::smatch m;
//    //if (std::regex_search(input, m, r)) {
//    //    //assert(m.size() == 3);
//    //    //assert(m[0].str() == "PQR:2;");                // Entire match
//    //    //assert(m[1].str() == "PQR");                   // Substring that matches 1st group
//    //    //assert(m[2].str() == "2");                     // Substring that matches 2nd group
//    //    //assert(m.prefix().str() == "ABC:1->   ");      // All before 1st character match
//    //    //assert(m.suffix().str() == ";;   XYZ:3<<<");   // All after last character match
//
//    //    for (std::string&& str : m) { // Alternatively. You can also do
//    //        std::cout << str << std::endl;
//    //    }
//    //}
//
//    std::cout << "\n\n";
//    system("pause");
//    return 0;
//}