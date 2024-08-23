#pragma once
#include "../Common/Config.h"
#if ENGINE_TYPE == MEDIA_ENDIGNE_LOGIC
#include "AvReaderDxgiEffect.h"
#include <Helpers/Dx/DxDevice.h>
#include <mfreadwrite.h>
#include <mfobjects.h>

class AvReaderDxgiManager {
public:
    AvReaderDxgiManager(H::Dx::DxDeviceSafeObj* dxDeviceSafeObj);
    virtual ~AvReaderDxgiManager();

    void SetAttributes(IMFAttributes* attr);
    std::unique_ptr<IAvReaderEffect> CreateEffect(IMFMediaType* input);

private:
    H::Dx::DxDeviceSafeObj* dxDeviceSafeObj;
#if AvReaderDxgiManager_NEW_LOGIC
    H::Dx::DxDeviceSafeObj dxDeviceForVideoRenderSafeObj;
#endif
};

#elif ENGINE_TYPE == DX_PLAYER_RENDER

#include "AvReaderDxgiEffect.h"
#include "Common/DeviceResources.h"
#include <mfreadwrite.h>
#include <mfobjects.h>

class AvReaderDxgiManager {
public:
    AvReaderDxgiManager(DX::DeviceResources* dxDevice);
    virtual ~AvReaderDxgiManager();

    void SetAttributes(IMFAttributes* attr);
    std::unique_ptr<IAvReaderEffect> CreateEffect(IMFMediaType* input);

private:
    DX::DeviceResources* dxDeviceSafeObj;
#if AvReaderDxgiManager_NEW_LOGIC
    DX::DeviceResources dxDeviceForVideoRenderSafeObj;
#endif

    Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mfDxgiDeviceManager;
};
#endif