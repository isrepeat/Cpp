#include "pch.h"
#include "BmpSize.h"

#include <cmath>

const Structs::Float2 BmpSize::DefaultDpi = Structs::Float2(96.f, 96.f);

BmpSize::BmpSize()
    : logicalSize(0.f, 0.f)
    , logicalDpi(BmpSize::DefaultDpi)
{}

BmpSize::BmpSize(const Structs::Float2 &logicalSize, ID2D1DeviceContext* d2dCtx)
    : logicalSize(logicalSize)
{
    Structs::Float2 dpi = BmpSize::DefaultDpi;
    d2dCtx->GetDpi(&dpi.x, &dpi.y);
    this->logicalDpi = dpi;
}

BmpSize::BmpSize(const Structs::Float2 &logicalSize, const Structs::Float2 &logicalDpi)
    : logicalSize(logicalSize)
    , logicalDpi(logicalDpi)
{}

bool BmpSize::operator==(const BmpSize &other) const {
    bool equ =
        this->logicalSize.x == other.logicalSize.x &&
        this->logicalSize.y == other.logicalSize.y &&
        this->logicalDpi.x == other.logicalDpi.x &&
        this->logicalDpi.y == other.logicalDpi.y;

    return equ;
}

bool BmpSize::operator!=(const BmpSize &other) const {
    return !BmpSize::operator==(other);
}

Structs::Float2 BmpSize::GetLogicalSize() const {
    return this->logicalSize;
}

void BmpSize::SetLogicalSize(const Structs::Float2 &v) {
    this->logicalSize = v;
}

float BmpSize::GetLogicalWidth() const {
    return this->logicalSize.x;
}

//void BmpSize::SetLogicalWidth(float v) {
//    this->logicalSize.x = v;
//}

float BmpSize::GetLogicalHeight() const {
    return this->logicalSize.y;
}

//void BmpSize::SetLogicalHeight(float v) {
//    this->logicalSize.y = v;
//}

Structs::Float2 BmpSize::GetLogicalDpi() const {
    return this->logicalDpi;
}

//void BmpSize::SetLogicalDpi(const Structs::Float2 &v) {
//    this->logicalDpi = v;
//}

float BmpSize::GetLogicalDpiX() const {
    return this->logicalDpi.x;
}

//void BmpSize::SetLogicalDpiX(float v) {
//    this->logicalDpi.x = v;
//}

float BmpSize::GetLogicalDpiY() const {
    return this->logicalDpi.y;
}

//void BmpSize::SetLogicalDpiY(float v) {
//    this->logicalDpi.y = v;
//}

Structs::Float2 BmpSize::GetPhysicalSize() const {
    Structs::Float2 size;

    size.x = this->GetPhysicalWidth();
    size.y = this->GetPhysicalHeight();

    return size;
}

void BmpSize::SetPhysicalSize(const Structs::Float2 &v) {
    this->logicalSize.x = v.x * (BmpSize::DefaultDpi.x / this->logicalDpi.x);
    this->logicalSize.y = v.y * (BmpSize::DefaultDpi.y / this->logicalDpi.y);
}

//void BmpSize::SetPhysicalSize(const Structs::Float2 &v, const Structs::Float2 &logicalDpi) {
//    this->SetPhysicalWidth(v.x, logicalDpi.x);
//    this->SetPhysicalHeight(v.y, logicalDpi.y);
//}

float BmpSize::GetPhysicalWidth() const {
    float v = this->logicalSize.x * (this->logicalDpi.x / BmpSize::DefaultDpi.x);
    return v;
}

//void BmpSize::SetPhysicalWidth(float v, float dpiX) {
//    this->logicalDpi.x = dpiX;
//    this->logicalSize.x = v * (BmpSize::DefaultDpi.x / this->logicalDpi.x);
//}

float BmpSize::GetPhysicalHeight() const {
    float v = this->logicalSize.y * (this->logicalDpi.y / BmpSize::DefaultDpi.y);
    return v;
}

//void BmpSize::SetPhysicalHeight(float v, float dpiY) {
//    this->logicalDpi.y = dpiY;
//    this->logicalSize.y = v * (BmpSize::DefaultDpi.y / this->logicalDpi.y);
//}

Structs::Uint2 BmpSize::GetLogicalSizeUint() const {
    Structs::Uint2 size;

    size.x = this->GetLogicalWidthUint();
    size.y = this->GetLogicalHeightUint();

    return size;
}

uint32_t BmpSize::GetLogicalWidthUint() const {
    float tmp = this->GetLogicalWidth();
    uint32_t v = (uint32_t)std::ceilf(tmp);
    return v;
}

uint32_t BmpSize::GetLogicalHeightUint() const {
    float tmp = this->GetLogicalHeight();
    uint32_t v = (uint32_t) std::ceilf(tmp);
    return v;
}

Structs::Uint2 BmpSize::GetPhysicalSizeUint() const {
    Structs::Uint2 size;

    size.x = this->GetPhysicalWidthUint();
    size.y = this->GetPhysicalHeightUint();

    return size;
}

uint32_t BmpSize::GetPhysicalWidthUint() const {
    float tmp = this->GetPhysicalWidth();
    uint32_t v = (uint32_t)std::ceilf(tmp);
    return v;
}

uint32_t BmpSize::GetPhysicalHeightUint() const {
    float tmp = this->GetPhysicalHeight();
    uint32_t v = (uint32_t)std::ceilf(tmp);
    return v;
}