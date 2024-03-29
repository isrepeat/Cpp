#pragma once
#include <regex>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

namespace H {
	// [Print]
	template<typename T, typename... Args>
	int StringPrintFormat(T* buffer, size_t size, const T* format, Args... args) {
		if constexpr (std::is_same_v<T, wchar_t>) {
			return std::swprintf(buffer, size, format, args...);
		}
		else {
			return std::snprintf(buffer, size, format, args...);
		}
	}

	template<typename T, typename... Args>
	std::basic_string<T> StringFormat(const std::basic_string<T>& format, Args... args) {
		size_t size = StringPrintFormat<T>(nullptr, 0, format.c_str(), std::forward<Args>(args)...) + 1; // Extra space for '\0'
		if (size <= 0) {
			throw std::runtime_error("Error during formatting.");
		}
		std::unique_ptr<T[]> buf(new T[size]);
		StringPrintFormat<T>(buf.get(), size, format.c_str(), std::forward<Args>(args)...);
		return std::basic_string<T>(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	}

	// Overload for raw char/wchar_t (template deduction can't work with type conversion)
	template<typename T, typename... Args>
	std::basic_string<T> StringFormat(const T* format, Args... args) {
		return StringFormat(std::basic_string<T>(format), std::forward<Args>(args)...);
	}

	// TODO: add template with args ...
	void DebugOutput(const std::wstring& msg);
	void DebugOutput(const std::string& msg);


	std::string to_lower(std::string str);
	std::string to_upper(std::string str);

	// [Regex]
	struct RegexMatchResult {
		RegexMatchResult(const std::wsmatch& match);
		std::vector<std::wstring> matches;
		std::wstring preffix;
		std::wstring suffix;
	};

	std::vector<RegexMatchResult> GetRegexMatches(const std::wstring& text, const std::wregex& rx);
	bool FindInsideTagWithRegex(const std::wstring& text, const std::wstring& tag, const std::wregex& innerRx);
	bool FindInsideAnyTagWithRegex(const std::wstring& text, const std::wregex& innerRx);
}