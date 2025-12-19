#pragma once
#include "IAvReaderEffect.h"
#include <Helpers/MediaFoundation/MFInclude.h>
#include <Helpers/Dx/DxSharedTexture.h>
#include <Helpers/Dx/DxgiDeviceLock.h>
#include <Helpers/Dx/DxDevice.h>
#include <vector>

#define AvReaderDxgiManager_NEW_LOGIC 1

class AvReaderDxgiEffect : public IAvReaderEffect {
public:
#if AvReaderDxgiManager_NEW_LOGIC
    AvReaderDxgiEffect(IMFMediaType* input, H::Dx::DxDeviceSafeObj* dxDeviceSafeObj, Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mfDxgiDeviceManager);
#else
    AvReaderDxgiEffect(IMFMediaType* input, H::Dx::DxDeviceSafeObj* dxDeviceSafeObj);
#endif
    virtual ~AvReaderDxgiEffect();

    std::unique_ptr<MF::MFVideoSample> Process(std::unique_ptr<MF::MFSample> mfSample) override;

private:
    H::Dx::DxDeviceSafeObj* dxDeviceSafeObj;
#if AvReaderDxgiManager_NEW_LOGIC
    Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mfDxgiDeviceManager;
    std::vector<std::unique_ptr<H::Dx::DxSharedTexture>> sharedTextures;
    size_t sharedTextureCursor = 0;
    D3D11_TEXTURE2D_DESC sharedTextureDesc = {};
#endif
    std::unique_ptr<H::Dx::DxSharedTexture> sharedTexture;
    DirectX::XMUINT2 videoSize;
};
