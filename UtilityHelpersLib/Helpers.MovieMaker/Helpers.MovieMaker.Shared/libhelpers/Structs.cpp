#include "pch.h"
#include "Structs.h"

namespace Structs {
    Float2::Float2()
        : x(0.0f), y(0.0f) {}

    Float2::Float2(float x, float y)
        : x(x), y(y) {}




    Uint2::Uint2()
        : x(0), y(0) {}

    Uint2::Uint2(uint32_t x, uint32_t y)
        : x(x), y(y) {}




    Rgba::Rgba()
        : val(0) {}

    Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        : r(r), g(g), b(b), a(a) {}

    Rgba::Rgba(float r, float g, float b, float a)
        : r((uint8_t)(r / 255.0f)), g((uint8_t)(g / 255.0f)),
        b((uint8_t)(b / 255.0f)), a((uint8_t)(a / 255.0f)) {}




    RgbaF::RgbaF()
        : r(0.0f), g(0.0f), b(0.0f), a(0.0f) {}

    RgbaF::RgbaF(const float *m)
        : r(m[0]), g(m[1]), b(m[2]), a(m[3]) {}

    RgbaF::RgbaF(const std::array<float, 4> &ar)
        : ar(ar) {}

    RgbaF::RgbaF(float r, float g, float b, float a)
        : r(r), g(g), b(b), a(a) {}




    Rect::Rect()
        : left(0.0f), top(0.0f), right(0.0f), bottom(0.0f)
    {}

    Rect::Rect(float left, float top, float right, float bottom)
        : left(left), top(top), right(right), bottom(bottom)
    {}
}