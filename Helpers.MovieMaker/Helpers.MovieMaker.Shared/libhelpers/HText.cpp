#include "pch.h"
#include "HText.h"
#include "Text\UriCodec.h"

namespace H {
	std::string Text::UriDecode(const void *src, size_t length) {
		return UriCodec::Decode(static_cast<const char *>(src), length);
	}

	std::string Text::UriEncode(const void *src, size_t length) {
		return UriCodec::Encode(static_cast<const char *>(src), length);
	}

	std::wstring Text::ConvertUTF8ToWString(const std::string &utf8) {
#ifdef HAVE_VISUAL_STUDIO
		std::wstring wpath(utf8.length() + 1, '\0');

		// Runtime is modern, narrow calls are widened inside CRT using CP_ACP codepage.
		UINT codepage = CP_UTF8;

		(void)MultiByteToWideChar(codepage, 0, utf8.data(), (int)utf8.length(), &wpath[0], (int)wpath.size());

		auto zeroEnd = wpath.find(L'\0');

		if (zeroEnd != std::wstring::npos) {
			wpath.erase(wpath.begin() + zeroEnd, wpath.end());
		}
#elif
#error Add your implementation
#endif

		return wpath;
	}

	std::string Text::ConvertWStringToUTF8(const std::wstring &s) {
		std::string result;
		if (s.length() == 0) {
			return result;
		}
#ifdef HAVE_VISUAL_STUDIO
		UINT codepage = CP_UTF8;

		int requiredLength = WideCharToMultiByte(codepage, WC_ERR_INVALID_CHARS, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);

		result.resize(requiredLength);

		if (WideCharToMultiByte(codepage, 0, s.data(), (int)s.size(), const_cast<char *>(result.data()), (int)result.size(), nullptr, nullptr) == 0) {
			throw std::exception("Unable to convert Platform::String to std::string!");
		}
#elif
#error Add your implementation
#endif

		// success
		return result;
	}

#if HAVE_WINRT == 1
	std::string Text::ConvertToUTF8(Platform::String ^v) {
		std::string result;
		UINT codepage = CP_UTF8;

		int requiredLength = WideCharToMultiByte(codepage, 0, v->Data(), v->Length(), nullptr, 0, nullptr, nullptr);

		result.resize(requiredLength);

		if (WideCharToMultiByte(codepage, 0, v->Data(), (int)v->Length(), const_cast<char *>(result.data()), (int)result.size(), nullptr, nullptr) == 0) {
			throw std::exception("Unable to convert Platform::String to std::string!");
		}

		return result;
	}
#endif

	std::wstring Text::CreateStringParams(const std::vector<std::pair<std::wstring, std::wstring>>& params) {
		std::wstring stringParams = L"";

		for (auto& pair : params) {
			stringParams += L" " + pair.first + L" '" + pair.second + L"'"; // wrap second(value) to quotes '' for containins spaces etc
		}

		return stringParams;
	}


	std::wstring Text::ReplaceSubStr(std::wstring src, const std::wstring& subStr, const std::wstring& newStr) {
		// TODO: handle case when subStr not found in src (or subStr > src)
		return src.replace(src.find(subStr), subStr.length(), newStr);
	}


	// CP_ACP - default code page
	std::string Text::WStrToStr(const std::wstring& wstr, int codePage) {
		if (wstr.size() == 0)
			return std::string{};

		int sz = WideCharToMultiByte(codePage, 0, wstr.c_str(), -1, 0, 0, 0, 0);
		std::string res(sz, 0);
		WideCharToMultiByte(codePage, 0, wstr.c_str(), -1, &res[0], sz, 0, 0);
		return res.c_str(); // To delete '\0' use .c_str()
	}

	std::wstring Text::StrToWStr(const std::string& str, int codePage) {
		if (str.size() == 0)
			return std::wstring{};

		int sz = MultiByteToWideChar(codePage, 0, str.c_str(), -1, 0, 0);
		std::wstring res(sz, 0);
		MultiByteToWideChar(codePage, 0, str.c_str(), -1, &res[0], sz);
		return res.c_str(); // To delete '\0' use .c_str()
	}

	std::wstring Text::VecToWStr(const std::vector<wchar_t>& vec) {
		return std::wstring{ vec.begin(), vec.end() }.c_str(); // .c_str() remove trailing '\0'
	}

	std::string Text::VecToStr(const std::vector<char>& vec) {
		return std::string{ vec.begin(), vec.end() }.c_str(); // .c_str() remove trailing '\0'
	}

	void Text::BreakNetUrl(const std::wstring &url, std::wstring *protocol, std::wstring *name, std::wstring *port, std::wstring *path) {
		if (protocol || name || port || path) {
			std::wstring buffer;
			size_t start = 0, i = 0;
			bool havePort = false;

			while (url[i] == ':' || url[i] == '/') {
				i++;
			}
			start = i;

			while (i < url.size()) {
				if (url[i] == ':' || url[i] == '/') {
					buffer.push_back(url[i]);
				}
				else {
					if (!buffer.empty()) {
						if (buffer.find(L":/") == 0) {
							// have protocol
							if (protocol) {
								*protocol = std::wstring(url.begin() + start, url.begin() + (i - buffer.size()));
							}
							if (!(name || port || path)) {
								break;
							}
						}
						else if (buffer[0] == ':') {
							// port start
							if (name) {
								*name = std::wstring(url.begin() + start, url.begin() + (i - buffer.size()));
							}
							havePort = true;
							if (!(port || path)) {
								break;
							}
						}
						else if (buffer[0] == '/') {
							// name+path or port+path
							if (havePort) {
								if (port) {
									*port = std::wstring(url.begin() + start, url.begin() + (i - buffer.size()));
								}
							}
							else {
								if (name) {
									*name = std::wstring(url.begin() + start, url.begin() + (i - buffer.size()));
								}
							}
							start = i;
							if (path) {
								*path = std::wstring(url.begin() + start, url.end());
							}
							break;
						}

						buffer.clear();
						start = i;
					}
				}
				i++;
			}
		}
	}

    std::wstring Text::RemoveSlashes(std::wstring path) {
        if (!path.empty()) {
            if (path.front() == L'\\') {
                path.erase(path.begin());
            }

            if (path.back() == L'\\') {
                path.pop_back();
            }
        }

        return path;
    }

    void Text::BreakFileName(const std::wstring &fileName, std::wstring &name, std::wstring &extension) {
        auto lastDotPos = fileName.find_last_of('.');

        if (lastDotPos == std::wstring::npos) {
            name = fileName;
            extension = L"";
        }
        else {
            name = fileName.substr(0, lastDotPos);
            extension = fileName.substr(lastDotPos);
        }
    }

    void Text::BreakPath(const std::wstring &path, std::wstring &basePath, std::wstring &name) {
        auto lastSlashPos = path.find_last_of(L'\\');

        if (lastSlashPos == std::wstring::npos) {
            basePath = L"";
            name = path;
        }
        else {
            basePath = path.substr(0, lastSlashPos + 1);
            name = path.substr(lastSlashPos + 1);
        }
    }

	void Text::DebugOutput(const std::wstring& msg) {
		OutputDebugStringW((msg + L"\n").c_str());
	}

	void Text::DebugOutput(const std::string& msg) {
		OutputDebugStringA((msg + "\n").c_str());
	}
}