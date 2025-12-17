#pragma once
#include "IAvDxBufferFactory.h"

class AvDxgiManagerBufferFactory : public IAvDxBufferFactory {
public:
    AvDxgiManagerBufferFactory(IMFDXGIDeviceManager *manager);

    virtual Microsoft::WRL::ComPtr<IMFMediaBuffer> CreateBuffer(ID3D11DeviceContext *d3dCtx, ID3D11Texture2D *tex) override;
    virtual void SetAttributes(IMFAttributes *attr) override;

private:
    Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> manager;
};