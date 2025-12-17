#pragma once
#include "common.h"
#include <system_error>
#include <string_view>
#include <algorithm>
#include <cwctype>
#include <string>
#include <memory>

namespace HELPERS_NS {
	template <typename T>
	struct StringDeductor {
		template <typename A> StringDeductor(A) {}
		using type = T;
	};

	// user-defined deduction guides:
	StringDeductor(const char*)->StringDeductor<char>;
	StringDeductor(const wchar_t*)->StringDeductor<wchar_t>;
	StringDeductor(std::string)->StringDeductor<char>;
	StringDeductor(std::wstring)->StringDeductor<wchar_t>;
	StringDeductor(std::string_view)->StringDeductor<char>;
	StringDeductor(std::wstring_view)->StringDeductor<wchar_t>;


	inline std::wstring ToLower(std::wstring wstr) {
		std::transform(wstr.begin(), wstr.end(), wstr.begin(), std::towlower);
		return wstr;
	}
	inline std::string ToLower(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), std::towlower);
		return str;
	}
	inline std::wstring ToUpper(std::wstring wstr) {
		std::transform(wstr.begin(), wstr.end(), wstr.begin(), std::towupper);
		return wstr;
	}
	inline std::string ToUpper(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), std::towupper);
		return str;
	}

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

	template<typename T>
	std::basic_string<T> GetStringByType(std::basic_string_view<char> s1, std::basic_string_view<wchar_t> s2) {
		return std::get<std::basic_string_view<T>>(std::tuple<std::basic_string_view<char>, std::basic_string_view<wchar_t>>(s1, s2));
	}

	template<typename T>
	static constexpr std::basic_string_view<T> GetStringViewByType(std::basic_string_view<char> s1, std::basic_string_view<wchar_t> s2) {
		return std::get<std::basic_string_view<T>>(std::tuple<std::basic_string_view<char>, std::basic_string_view<wchar_t>>(s1, s2));
	}
}

#define JOIN_STRING(A, B) A B // keep space between A and B

#define MAKE_STRING_T(T, str) HELPERS_NS::GetStringByType<T>(str, L""##str)
#define MAKE_STRING_VIEW_T(T, str) HELPERS_NS::GetStringViewByType<T>(str, L""##str)