#include "Regex.h"

namespace H {
    std::vector<RegexMatchResult> GetRegexMatches(const std::wstring& text, const std::wregex& rx) {
        std::vector<RegexMatchResult> matches;
        const std::wsregex_token_iterator end_i;
        for (std::wsregex_token_iterator i(text.cbegin(), text.cend(), rx); i != end_i; ++i) {
            std::wstring matchString = *i;
            std::wsmatch matchResult;
            if (std::regex_search(matchString, matchResult, rx)) {
                matches.push_back(matchResult);
            }
        }
        return matches;
    }

    bool FindInsideTagWithRegex(const std::wstring& text, const std::wstring& tag, const std::wregex& innerRx) {
        const std::wregex rx(L"([^<]*)<" + tag + L"[^>]*>(.+?)<[/]" + tag + L">([^<]*)");

        auto matches = GetRegexMatches(text, rx);
        for (auto& match : matches) {
            std::wsmatch matchResult;
            if (std::regex_search(match.matches[2], matchResult, innerRx)) {
                return true;
            }
        }
        return false;
    }
}