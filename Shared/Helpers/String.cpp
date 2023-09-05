#pragma once
#include "String.h"
#include <Windows.h>

namespace H {
	void DebugOutput(const std::wstring& msg) {
		OutputDebugStringW((L"[dbg] " + msg+L"\n").c_str());
	}

	void DebugOutput(const std::string& msg) {
		OutputDebugStringA(("[dbg] " + msg+"\n").c_str());
	}


    std::string to_lower(std::string str) {
        for (char& ch : str) {
            ch = tolower(ch);
        }
        return str;
    }

    std::string to_upper(std::string str) {
        for (char& ch : str) {
            ch = toupper(ch);
        }
        return str;
    }


    RegexMatchResult::RegexMatchResult(const std::wsmatch& match) {
        for (auto& res : match) {
            matches.push_back(res);
        }
        preffix = match.prefix();
        suffix = match.suffix();
    }

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

    bool FindInsideAnyTagWithRegex(const std::wstring& text, const std::wregex& innerRx) {
        const std::wstring anyTag = L"[^>]*";
        const std::wregex rx(L"([^<]*)<" + anyTag + L">(.+?)<[/]" + anyTag + L">([^<]*)");

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