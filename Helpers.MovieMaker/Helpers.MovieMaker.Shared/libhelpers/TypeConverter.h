#pragma once
#include "config.h"
#include "Structs.h"

#include <d2d1.h>
#include <string>
#include <cstdint>
#include <array>
#include <directxmath.h>

template<class D, class S>
struct TypeConverter {
	static D Convert(const S &v) {
		static_assert(false, "Not implemented");
		return D();
	}
};

// std::wstring <-> std::string
template<> struct TypeConverter<std::wstring, std::string> {
	static std::wstring Convert(const std::string &v);
};

template<> struct TypeConverter<std::string, std::wstring> {
	static std::string Convert(const std::wstring &v);
};

// D2D1_SIZE_U -> D2D1_SIZE_F
template<> struct TypeConverter<D2D1_SIZE_F, D2D1_SIZE_U> {
	static D2D1_SIZE_F Convert(const D2D1_SIZE_U &v);
};

// Structs::Rgba <-> D2D1::ColorF
template<> struct TypeConverter<Structs::Rgba, D2D1::ColorF> {
	static Structs::Rgba Convert(const D2D1::ColorF &v);
};

template<> struct TypeConverter<D2D1::ColorF, Structs::Rgba> {
	static D2D1::ColorF Convert(const Structs::Rgba &v);
};

// Structs::Rgba <-> Structs::RgbaF
template<> struct TypeConverter<Structs::Rgba, Structs::RgbaF> {
	static Structs::Rgba Convert(const Structs::RgbaF &v);
};

template<> struct TypeConverter<Structs::RgbaF, Structs::Rgba> {
	static Structs::RgbaF Convert(const Structs::Rgba &v);
};

// DirectX::XMFLOAT3 <-> DirectX::XMFLOAT2
template<> struct TypeConverter<DirectX::XMFLOAT3, DirectX::XMFLOAT2> {
	static DirectX::XMFLOAT3 Convert(const DirectX::XMFLOAT2& v);
};

template<> struct TypeConverter<DirectX::XMFLOAT2, DirectX::XMFLOAT3> {
	static DirectX::XMFLOAT2 Convert(const DirectX::XMFLOAT3& v);
};

#if HAVE_WINRT == 1
// Windows::UI::Color <-> D2D1::ColorF
template<> struct TypeConverter<Windows::UI::Color, D2D1::ColorF> {
	static Windows::UI::Color Convert(const D2D1::ColorF &v);
};

template<> struct TypeConverter<D2D1::ColorF, Windows::UI::Color> {
	static D2D1::ColorF Convert(const Windows::UI::Color &v);
};

// Windows::UI::Color <-> D2D1_COLOR_F
template<> struct TypeConverter<Windows::UI::Color, D2D1_COLOR_F> {
	static Windows::UI::Color Convert(const D2D1_COLOR_F &v);
};

template<> struct TypeConverter<D2D1_COLOR_F, Windows::UI::Color> {
	static D2D1_COLOR_F Convert(const Windows::UI::Color &v);
};

// D2D1_COLOR_F <-> D2D1::ColorF
template<> struct TypeConverter<D2D1_COLOR_F, D2D1::ColorF> {
	static D2D1_COLOR_F Convert(const D2D1::ColorF &v);
};

template<> struct TypeConverter<D2D1::ColorF, D2D1_COLOR_F> {
	static D2D1::ColorF Convert(const D2D1_COLOR_F &v);
};

// Platform::String ^ <-> std::wstring
template<> struct TypeConverter<Platform::String ^, std::wstring> {
	static Platform::String ^Convert(const std::wstring &v);
};

template<> struct TypeConverter<std::wstring, Platform::String ^> {
	static std::wstring Convert(Platform::String ^v);
};

// Platform::String ^ <-> std::string
template<> struct TypeConverter<Platform::String ^, std::string> {
	static Platform::String ^Convert(const std::string &v);
};

template<> struct TypeConverter<std::string, Platform::String ^> {
	static std::string Convert(Platform::String ^v);
};

// Windows::Foundation::TimeSpan <-> int64_t
template<> struct TypeConverter<Windows::Foundation::TimeSpan, int64_t> {
	static  Windows::Foundation::TimeSpan Convert(const int64_t &v);
};

template<> struct TypeConverter<int64_t, Windows::Foundation::TimeSpan> {
	static int64_t Convert(Windows::Foundation::TimeSpan v);
};

// Windows::Foundation::Point <-> D2D1_Size_U
template<> struct TypeConverter<Windows::Foundation::Point, D2D1_SIZE_U> {
	static Windows::Foundation::Point Convert(D2D1_SIZE_U v);
};

template<> struct TypeConverter<D2D1_SIZE_U, Windows::Foundation::Point> {
	static D2D1_SIZE_U Convert(Windows::Foundation::Point v);
};
#endif