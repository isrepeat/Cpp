#include "pch.h"
#include <Helpers/System.h>
#include "AvReaderDxgiManager.h"
#include "AvReaderDxgiEffect.h"

#if ENGINE_TYPE == MEDIA_ENDIGNE_LOGIC
AvReaderDxgiManager::AvReaderDxgiManager(H::Dx::DxDeviceSafeObj* dxDeviceSafeObj)
    : dxDeviceSafeObj{ dxDeviceSafeObj }
{}

AvReaderDxgiManager::~AvReaderDxgiManager() {
}

void AvReaderDxgiManager::SetAttributes(IMFAttributes* attr) {
    HRESULT hr = S_OK;

#if AvReaderDxgiManager_NEW_LOGIC
    auto dxDeviceMf = this->mfDxDeviceSafeObj.Lock();
#else
    auto dxDeviceMf = this->dxDeviceSafeObj->Lock();
#endif
    dxDeviceMf->CreateMFDXGIDeviceManager();

    hr = attr->SetUnknown(MF_SOURCE_READER_D3D_MANAGER, dxDeviceMf->GetMFDXGIDeviceManager().Get());
    H::System::ThrowIfFailed(hr);

    /*hr = attr->SetUINT32(MF_READWRITE_D3D_OPTIONAL, FALSE);
    H::System::ThrowIfFailed(hr);*/

    // https://jeremiahmorrill.wordpress.com/2012/08/09/mediafoundation-for-metro/
    // among other things this flag allow use NV12 as uncompresseed type for MFSourceReader
    hr = attr->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE);
    H::System::ThrowIfFailed(hr);
}

std::unique_ptr<IAvReaderEffect> AvReaderDxgiManager::CreateEffect(IMFMediaType* input) {
#if AvReaderDxgiManager_NEW_LOGIC
    std::unique_ptr<IAvReaderEffect> res = std::make_unique<AvReaderDxgiEffect>(input, this->dxDeviceSafeObj, this->mfDxDeviceSafeObj.Lock()->GetMFDXGIDeviceManager());
#else
    std::unique_ptr<IAvReaderEffect> res = std::make_unique<AvReaderDxgiEffect>(input, this->dxDeviceSafeObj);
#endif
    return res;
}

#elif ENGINE_TYPE == DX_PLAYER_RENDER

AvReaderDxgiManager::AvReaderDxgiManager(DX::DeviceResources* dxDeviceSafeObj)
    : dxDeviceSafeObj{ dxDeviceSafeObj }
{}

AvReaderDxgiManager::~AvReaderDxgiManager() {
}

void AvReaderDxgiManager::SetAttributes(IMFAttributes* attr) {
    HRESULT hr = S_OK;

#if AvReaderDxgiManager_NEW_LOGIC
    auto dxDeviceMf = this->mfDxDeviceSafeObj.Lock();
#else
    auto dxDeviceMf = this->dxDeviceSafeObj->Lock();
#endif

    uint32_t resetToken = 0;
    hr = MFCreateDXGIDeviceManager(&resetToken, this->mfDxgiDeviceManager.ReleaseAndGetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = this->mfDxgiDeviceManager->ResetDevice(dxDeviceMf->GetD3DDevice(), resetToken);
    H::System::ThrowIfFailed(hr);

    hr = attr->SetUnknown(MF_SOURCE_READER_D3D_MANAGER, this->mfDxgiDeviceManager.Get());
    H::System::ThrowIfFailed(hr);

    /*hr = attr->SetUINT32(MF_READWRITE_D3D_OPTIONAL, FALSE);
    H::System::ThrowIfFailed(hr);*/

    // https://jeremiahmorrill.wordpress.com/2012/08/09/mediafoundation-for-metro/
    // among other things this flag allow use NV12 as uncompresseed type for MFSourceReader
    hr = attr->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE);
    H::System::ThrowIfFailed(hr);
}

std::unique_ptr<IAvReaderEffect> AvReaderDxgiManager::CreateEffect(IMFMediaType* input) { 
#if AvReaderDxgiManager_NEW_LOGIC
    std::unique_ptr<IAvReaderEffect> res = std::make_unique<AvReaderDxgiEffect>(input, this->dxDeviceSafeObj, this->mfDxgiDeviceManager);
#else
    std::unique_ptr<IAvReaderEffect> res = std::make_unique<AvReaderDxgiEffect>(input, this->dxDeviceSafeObj);
#endif
    return res;
}
#endif