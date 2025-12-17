#pragma once

#include <libhelpers/Dx/Dx.h>

class IMFOutputTexResize {
public:
    virtual ~IMFOutputTexResize() = default;

    virtual void Resize(
        ID3D11Device *dev,
        ID3D11DeviceContext *ctx,
        ID3D11Texture2D *dst,
        uint32_t dstSubResource,
        ID3D11Texture2D* src) = 0;
};
