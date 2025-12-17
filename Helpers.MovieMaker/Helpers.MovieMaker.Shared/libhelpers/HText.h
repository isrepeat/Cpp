#pragma once
#include "config.h"
#include "ToString.h"

#include <vector>
#include <string>

#ifdef HAVE_VISUAL_STUDIO
#include <Windows.h>
#endif

namespace H {
	class Text {
	public:
		static std::string UriDecode(const void *src, size_t length);

		static std::string UriEncode(const void *src, size_t length);

		static std::wstring ConvertUTF8ToWString(const std::string &utf8);

		static std::string ConvertWStringToUTF8(const std::wstring &s);

#if HAVE_WINRT == 1
		static std::string ConvertToUTF8(Platform::String ^v);
#endif

		static std::wstring CreateStringParams(const std::vector<std::pair<std::wstring, std::wstring>>& params);

		static std::wstring ReplaceSubStr(std::wstring src, const std::wstring& subStr, const std::wstring& newStr);
		static std::string WStrToStr(const std::wstring& wstr, int codePage = CP_ACP); // TODO: rewrite all with UTF_8 by default
		static std::wstring StrToWStr(const std::string& str, int codePage = CP_ACP);
		static std::wstring VecToWStr(const std::vector<wchar_t>& vec);
		static std::string VecToStr(const std::vector<char>& vec);


		static void BreakNetUrl(const std::wstring &url, std::wstring *protocol, std::wstring *name, std::wstring *port, std::wstring *path);

        static std::wstring RemoveSlashes(std::wstring path);
        static void BreakFileName(const std::wstring &fileName, std::wstring &name, std::wstring &extension);
        static void BreakPath(const std::wstring &path, std::wstring &basePath, std::wstring &name);

		template<class T> static std::string ToString(const T &v) {
			return ToStr::Do(v);
		}


		template<typename T, typename... Args>
		static int StringPrintFormat(T* buffer, size_t size, const T* format, Args... args) {
			if constexpr (std::is_same_v<T, wchar_t>) {
				return std::swprintf(buffer, size, format, args...);
			}
			else {
				return std::snprintf(buffer, size, format, args...);
			}
		}

		template<typename T, typename... Args>
		static std::basic_string<T> StringFormat(const std::basic_string<T>& format, Args... args) {
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
		static std::basic_string<T> StringFormat(const T* format, Args... args) {
			return StringFormat(std::basic_string<T>(format), std::forward<Args>(args)...);
		}

		// TODO: add template with args ...
		static void DebugOutput(const std::wstring& msg);
		static void DebugOutput(const std::string& msg);
	};
}