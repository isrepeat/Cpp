#include "pch.h"
#include "AvReaderDxgiEffect.h"
#include <Helpers/System.h>

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

    Microsoft::WRL::ComPtr<ID3D11Texture2D> dstTexture;
    std::shared_ptr<void> renderLease;
    {
        H::Dx::MFDXGIDeviceManagerLock mfDxgiDeviceManagerLock{ this->mfDxgiDeviceManager }; // it may block current thread when device lock / unlock

        Microsoft::WRL::ComPtr<ID3D11Device> mfD3dDevice;
        hr = mfDxgiDeviceManagerLock.LockDevice(mfD3dDevice.GetAddressOf());
        if (FAILED(hr)) {
            Dbreak;
        }

        D3D11_TEXTURE2D_DESC srcTextureDesc = {};
        mfSampleTexture->GetDesc(&srcTextureDesc);

        if (!this->sharedTexture) {
            this->sharedTexture = std::make_unique<H::Dx::DxSharedTexture>(
                srcTextureDesc,
                this->dxDeviceSafeObj->Lock()->GetD3DDevice(),
                mfD3dDevice
            );
        }

        // We intentionally receive the texture as a raw pointer and then Attach() it.
        //
        // CopyTextureShared() returns an ALREADY AddRef()'ed ID3D11Texture2D*.
        // Using raw + Attach() makes ownership transfer explicit and avoids:
        // - accidental extra AddRef() (which could happen with GetAddressOf()),
        // - relying on implicit ComPtr out-parameter semantics,
        // - issues if CopyTextureShared() changes its internal implementation.
        //
        // Attach() assumes ownership of the returned reference without calling AddRef().

        ID3D11Texture2D* dstTextureRaw = nullptr;
        this->sharedTexture->CopyTextureShared(
            &dstTextureRaw,
            &renderLease,
            mfSampleTexture
        );
        dstTexture.Attach(dstTextureRaw);
    }

    if (!dstTexture) {
        return nullptr;
    }

    MF::MFSample mfSampleCopy = *mfSample;
    mfSampleCopy.buffer = nullptr; // release original reference to IMFSample
    auto sample = std::make_unique<MF::MFVideoSample>(
        mfSampleCopy,
        dstTexture,
        std::move(renderLease)
    );
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
