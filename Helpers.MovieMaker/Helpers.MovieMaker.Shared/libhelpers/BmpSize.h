#pragma once
#include "Structs.h"

class BmpSize {
public:
    static const Structs::Float2 DefaultDpi;

    BmpSize();
    BmpSize(const Structs::Float2 &logicalSize, ID2D1DeviceContext* d2dCtx);
    BmpSize(const Structs::Float2 &logicalSize, const Structs::Float2 &logicalDpi);

    bool operator==(const BmpSize &other) const;
    bool operator!=(const BmpSize &other) const;

    Structs::Float2 GetLogicalSize() const;
    void SetLogicalSize(const Structs::Float2 &v);

    float GetLogicalWidth() const;
    //void SetLogicalWidth(float v);

    float GetLogicalHeight() const;
    //void SetLogicalHeight(float v);

    Structs::Float2 GetLogicalDpi() const;
    //void SetLogicalDpi(const Structs::Float2 &v);

    float GetLogicalDpiX() const;
    //void SetLogicalDpiX(float v);

    float GetLogicalDpiY() const;
    //void SetLogicalDpiY(float v);

    Structs::Float2 GetPhysicalSize() const;
    void SetPhysicalSize(const Structs::Float2 &v);
    //void SetPhysicalSize(const Structs::Float2 &v, const Structs::Float2 &logicalDpi);

    float GetPhysicalWidth() const;
    //void SetPhysicalWidth(float v, float dpiX);

    float GetPhysicalHeight() const;
    //void SetPhysicalHeight(float v, float dpiY);

    Structs::Uint2 GetLogicalSizeUint() const;

    uint32_t GetLogicalWidthUint() const;
    uint32_t GetLogicalHeightUint() const;

    Structs::Uint2 GetPhysicalSizeUint() const;

    uint32_t GetPhysicalWidthUint() const;
    uint32_t GetPhysicalHeightUint() const;

private:
    Structs::Float2 logicalSize;
    Structs::Float2 logicalDpi;
};