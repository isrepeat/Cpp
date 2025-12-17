#pragma once

#include <libhelpers\Dx\Dx.h>
#include <libhelpers\MediaFoundation\MFUser.h>

class IAvDxBufferFactory {
public:
    IAvDxBufferFactory() {}
    virtual ~IAvDxBufferFactory() {}

    virtual Microsoft::WRL::ComPtr<IMFMediaBuffer> CreateBuffer(ID3D11DeviceContext *d3dCtx, ID3D11Texture2D *tex) = 0;
    virtual void SetAttributes(IMFAttributes *attr) = 0;
};