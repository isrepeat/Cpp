#include "pch.h"
#include "TypeConverter.h"

#include <algorithm>

std::wstring TypeConverter<std::wstring, std::string>::Convert(const std::string& v) {
    std::wstring tmp(v.begin(), v.end());
    return tmp;
}

std::string TypeConverter<std::string, std::wstring>::Convert(const std::wstring& v) {
    std::string tmp;

    tmp.reserve(v.size());
    std::transform(std::begin(v), std::end(v), std::back_inserter(tmp), [](wchar_t ch) { return static_cast<char>(ch); });

    return tmp;
}

D2D1_SIZE_F TypeConverter<D2D1_SIZE_F, D2D1_SIZE_U>::Convert(const D2D1_SIZE_U& v) {
    D2D1_SIZE_F tmp;

    tmp.width = (float)v.width;
    tmp.height = (float)v.height;

    return tmp;
}

// Structs::Rgba <-> D2D1::ColorF
Structs::Rgba TypeConverter<Structs::Rgba, D2D1::ColorF>::Convert(const D2D1::ColorF& v) {
    Structs::RgbaF tmp(v.r, v.g, v.b, v.a);
    auto color = TypeConverter<Structs::Rgba, Structs::RgbaF>::Convert(tmp);

    return color;
}

D2D1::ColorF TypeConverter<D2D1::ColorF, Structs::Rgba>::Convert(const Structs::Rgba& v) {
    auto tmp = TypeConverter<Structs::RgbaF, Structs::Rgba>::Convert(v);
    auto color = D2D1::ColorF(tmp.r, tmp.g, tmp.b, tmp.a);

    return color;
}

// Structs::Rgba <-> Structs::RgbaF
Structs::Rgba TypeConverter<Structs::Rgba, Structs::RgbaF>::Convert(const Structs::RgbaF& v) {
    Structs::Rgba color;

    color.a = (uint8_t)(v.a * 255.0f);
    color.r = (uint8_t)(v.r * 255.0f);
    color.g = (uint8_t)(v.g * 255.0f);
    color.b = (uint8_t)(v.b * 255.0f);

    return color;
}

Structs::RgbaF TypeConverter<Structs::RgbaF, Structs::Rgba>::Convert(const Structs::Rgba& v) {
    const float Scale = 1.0f / 255.0f;
    Structs::RgbaF color;

    color.a = (float)v.a * Scale;
    color.r = (float)v.r * Scale;
    color.g = (float)v.g * Scale;
    color.b = (float)v.b * Scale;

    return color;
}

DirectX::XMFLOAT3 TypeConverter<DirectX::XMFLOAT3, DirectX::XMFLOAT2>::Convert(const DirectX::XMFLOAT2& v) {
    return {v.x, v.y, 1};
}

DirectX::XMFLOAT2 TypeConverter<DirectX::XMFLOAT2, DirectX::XMFLOAT3>::Convert(const DirectX::XMFLOAT3& v) {
    return {v.x, v.y};
}

#if HAVE_WINRT == 1
Windows::UI::Color TypeConverter<Windows::UI::Color, D2D1::ColorF>::Convert(const D2D1::ColorF& v) {
    Windows::UI::Color color;

    color.A = (uint8_t)(v.a * 255.0f);
    color.R = (uint8_t)(v.r * 255.0f);
    color.G = (uint8_t)(v.g * 255.0f);
    color.B = (uint8_t)(v.b * 255.0f);

    return color;
}

D2D1::ColorF TypeConverter<D2D1::ColorF, Windows::UI::Color>::Convert(const Windows::UI::Color& v) {
    const float Scale = 1.0f / 255.0f;
    float a, r, g, b;

    a = (float)v.A * Scale;
    r = (float)v.R * Scale;
    g = (float)v.G * Scale;
    b = (float)v.B * Scale;

    auto compColor = Windows::UI::Colors::Transparent;
    if (v.A == compColor.A && v.R == compColor.R && v.G == compColor.G && v.B == compColor.B) {
        a = r = g = b = 0.0f;
    }

    return D2D1::ColorF(r, g, b, a);
};

Windows::UI::Color TypeConverter<Windows::UI::Color, D2D1_COLOR_F>::Convert(const D2D1_COLOR_F& v) {
    D2D1::ColorF tmp(v.r, v.g, v.b, v.a);
    return TypeConverter<Windows::UI::Color, D2D1::ColorF>::Convert(tmp);
}

D2D1_COLOR_F TypeConverter<D2D1_COLOR_F, Windows::UI::Color>::Convert(const Windows::UI::Color& v) {
    D2D1_COLOR_F res = TypeConverter<D2D1::ColorF, Windows::UI::Color>::Convert(v);
    return res;
}

D2D1_COLOR_F TypeConverter<D2D1_COLOR_F, D2D1::ColorF>::Convert(const D2D1::ColorF& v) {
    D2D1_COLOR_F color;

    color.a = v.a;
    color.r = v.r;
    color.g = v.g;
    color.b = v.b;

    return color;
}

D2D1::ColorF TypeConverter<D2D1::ColorF, D2D1_COLOR_F>::Convert(const D2D1_COLOR_F& v) {
    D2D1::ColorF color(v.r, v.g, v.b, v.a);
    return color;
}

Platform::String^ TypeConverter<Platform::String^, std::wstring>::Convert(const std::wstring& v) {
    Platform::String^ tmp = ref new Platform::String(v.data(), (int)v.size());
    return tmp;
}

std::wstring TypeConverter<std::wstring, Platform::String^>::Convert(Platform::String^ v) {
    std::wstring tmp(v->Data(), v->Length());
    return tmp;
}

Platform::String^ TypeConverter<Platform::String^, std::string>::Convert(const std::string& v) {
    std::wstring tmpWStr(v.begin(), v.end());
    Platform::String^ tmp = ref new Platform::String(tmpWStr.data(), (int)tmpWStr.size());
    return tmp;
}

std::string TypeConverter<std::string, Platform::String^>::Convert(Platform::String^ v) {
    std::wstring tmpWStr(v->Data(), v->Length());
    auto tmp = TypeConverter<std::string, std::wstring>::Convert(tmpWStr);
    return tmp;
}

Windows::Foundation::TimeSpan TypeConverter<Windows::Foundation::TimeSpan, int64_t>::Convert(const int64_t& v) {
    Windows::Foundation::TimeSpan res;
    res.Duration = v;
    return res;
}

int64_t TypeConverter<int64_t, Windows::Foundation::TimeSpan>::Convert(Windows::Foundation::TimeSpan v) {
    return v.Duration;
}

Windows::Foundation::Point TypeConverter<Windows::Foundation::Point, D2D1_SIZE_U>::Convert(D2D1_SIZE_U v) {
    Windows::Foundation::Point res;
    res.X = static_cast<float>(v.width);
    res.Y = static_cast<float>(v.height);
    return res;
}

D2D1_SIZE_U TypeConverter<D2D1_SIZE_U, Windows::Foundation::Point>::Convert(Windows::Foundation::Point v) {
    D2D1_SIZE_U res = { static_cast<uint32_t>(v.X), static_cast<uint32_t>(v.Y) };
    return res;
}
#endif