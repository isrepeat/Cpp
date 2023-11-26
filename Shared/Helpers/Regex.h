#pragma once
#include <regex>
#include <vector>
#include <string>

namespace H {
    struct RegexMatchResult {
        RegexMatchResult(const std::wsmatch& match) {
            for (auto& res : match) {
                matches.push_back(res);
            }

            preffix = match.prefix();
            suffix = match.suffix();
        }

        std::vector<std::wstring> matches;
        std::wstring preffix;
        std::wstring suffix;
    };

    std::vector<RegexMatchResult> GetRegexMatches(const std::wstring& text, const std::wregex& rx);
    bool FindInsideTagWithRegex(const std::wstring& text, const std::wstring& tag, const std::wregex& innerRx);
}