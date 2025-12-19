#include "pch.h"
#include "AvReaderDxgiEffect.h"
#include <Helpers/System.h>
#include <cstring>

#if AvReaderDxgiManager_NEW_LOGIC
AvReaderDxgiEffect::AvReaderDxgiEffect(IMFMediaType* input, H::Dx::DxDeviceSafeObj* dxDeviceSafeObj, Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mfDxgiDeviceManager)
    : dxDeviceSafeObj{ dxDeviceSafeObj }
    , mfDxgiDeviceManager{ mfDxgiDeviceManager } {
    HRESULT hr = S_OK;

    hr = MFGetAttributeSize(input, MF_MT_FRAME_SIZE, &this->videoSize.x, &this->videoSize.y);
    H::System::ThrowIfFailed(hr);
}

#else

AvReaderDxgiEffect::AvReaderDxgiEffect(IMFMediaType* input, H::Dx::DxDeviceSafeObj* dxDeviceSafeObj)
    : dxDeviceSafeObj{ dxDeviceSafeObj } {
    HRESULT hr = S_OK;

    hr = MFGetAttributeSize(input, MF_MT_FRAME_SIZE, &this->videoSize.x, &this->videoSize.y);
    H::System::ThrowIfFailed(hr);
}
#endif

AvReaderDxgiEffect::~AvReaderDxgiEffect() {
}

std::unique_ptr<MF::MFVideoSample> AvReaderDxgiEffect::Process(std::unique_ptr<MF::MFSample> mfSample) {
    HRESULT hr = S_OK;

#if AvReaderDxgiManager_NEW_LOGIC
    Microsoft::WRL::ComPtr<IMFDXGIBuffer> dxgiBuffer;
    hr = mfSample->buffer.As(&dxgiBuffer);
    H::System::ThrowIfFailed(hr);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> mfSampleTexture;
    hr = dxgiBuffer->GetResource(IID_PPV_ARGS(&mfSampleTexture));
    H::System::ThrowIfFailed(hr);

    H::Dx::MFDXGIDeviceManagerLock mfDxgiDeviceManagerLock{ this->mfDxgiDeviceManager }; // it may block current thread when device lock / unlock

    Microsoft::WRL::ComPtr<ID3D11Device> mfD3dDevice;
    hr = mfDxgiDeviceManagerLock.LockDevice(mfD3dDevice.GetAddressOf());
    if (FAILED(hr)) {
        Dbreak;
    }

    D3D11_TEXTURE2D_DESC srcTextureDesc = {};
    mfSampleTexture->GetDesc(&srcTextureDesc);

    // Recreate pool if description changed or not initialized.
    if (this->sharedTextures.empty() || memcmp(&this->sharedTextureDesc, &srcTextureDesc, sizeof(D3D11_TEXTURE2D_DESC)) != 0) {
        this->sharedTextures.clear();
        this->sharedTextureCursor = 0;
        this->sharedTextureDesc = srcTextureDesc;

        constexpr size_t poolSize = 3;
        for (size_t i = 0; i < poolSize; ++i) {
            this->sharedTextures.push_back(std::make_unique<H::Dx::DxSharedTexture>(srcTextureDesc, this->dxDeviceSafeObj->Lock()->GetD3DDevice(), mfD3dDevice));
        }
    }

    const size_t poolIndex = this->sharedTextureCursor;
    auto& sharedTexture = *this->sharedTextures[poolIndex];
    this->sharedTextureCursor = (this->sharedTextureCursor + 1) % this->sharedTextures.size();

    sharedTexture.CopyFrom(mfSampleTexture);
    auto dstTexture = sharedTexture.GetDstTexture();

    MF::MFSample mfSampleCopy = *mfSample;
    mfSampleCopy.buffer = nullptr; // release original reference to IMFSample
    auto sample = std::make_unique<MF::MFVideoSample>(mfSampleCopy, dstTexture);
    return sample;

#else

    Microsoft::WRL::ComPtr<IMFDXGIBuffer> dxgiBuffer;
    hr = mfSample->buffer.As(&dxgiBuffer);
    H::System::ThrowIfFailed(hr);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> mfSampleTexture;
    hr = dxgiBuffer->GetResource(IID_PPV_ARGS(&mfSampleTexture));
    H::System::ThrowIfFailed(hr);

    auto sample = std::make_unique<MF::MFVideoSample>(*mfSample, mfSampleTexture);
    return sample;
#endif
}
