#pragma once
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

    Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mfDxgiDeviceManager;
};