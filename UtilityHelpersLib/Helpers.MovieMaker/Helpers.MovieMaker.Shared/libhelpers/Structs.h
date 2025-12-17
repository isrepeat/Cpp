#pragma once

#include <array>
#include <cstdint>

namespace Structs {
    struct Float2 {
        union {
            float m[2];

            struct {
                float x;
                float y;
            };
        };

        Float2();
        Float2(float x, float y);
    };




    struct Uint2 {
        union {
            uint32_t m[2];

            struct {
                uint32_t x;
                uint32_t y;
            };
        };

        Uint2();
        Uint2(uint32_t x, uint32_t y);
    };




    struct Rgba {
        union {
            uint8_t m[4];
            uint32_t val;

            struct {
                uint8_t r;
                uint8_t g;
                uint8_t b;
                uint8_t a;
            };
        };

        Rgba();
        Rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
        Rgba(float r, float g, float b, float a = 1.0f);
    };

    struct RgbaF {
        union {
            float m[4];
            std::array<float, 4> ar;

            struct {
                float r;
                float g;
                float b;
                float a;
            };
        };

        RgbaF();
        RgbaF(const float *m);
        RgbaF(const std::array<float, 4> &ar);
        RgbaF(float r, float g, float b, float a = 1.0f);
    };

    struct Rect {
        union {
            float m[4];

            struct {
                float left;
                float top;
                float right;
                float bottom;
            };

            struct {
                Float2 pt[2];
            };
        };

        Rect();
        Rect(float left, float top, float right, float bottom);
    };
}