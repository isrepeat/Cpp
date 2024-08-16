#include "AvReaderDxgiEffect.h"
#include <Helpers/System.h>


#if AvReaderDxgiManager_NEW_LOGIC
AvReaderDxgiEffect::AvReaderDxgiEffect(IMFMediaType* input, H::Dx::DxDeviceSafeObj* dxDeviceSafeObj, Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mfDxgiDeviceManager)
    : dxDeviceSafeObj{ dxDeviceSafeObj }
    , mfDxgiDeviceManager{ mfDxgiDeviceManager }
{
    HRESULT hr = S_OK;

    hr = MFGetAttributeSize(input, MF_MT_FRAME_SIZE, &this->videoSize.x, &this->videoSize.y);
    H::System::ThrowIfFailed(hr);
}

#else

AvReaderDxgiEffect::AvReaderDxgiEffect(IMFMediaType* input, H::Dx::DxDeviceSafeObj* dxDeviceSafeObj)
    : dxDeviceSafeObj{ dxDeviceSafeObj }
{
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

    DWORD mfSampleTotalLength = 0;
    hr = mfSample->pSample->GetTotalLength(&mfSampleTotalLength);

    Microsoft::WRL::ComPtr<IMFMediaBuffer> mfSampleBuffer;
    hr = mfSample->pSample->ConvertToContiguousBuffer(&mfSampleBuffer);
    H::System::ThrowIfFailed(hr);

    Microsoft::WRL::ComPtr<IMFDXGIBuffer> mfSampleDxgiBuffer;
    hr = mfSampleBuffer.As(&mfSampleDxgiBuffer);
    H::System::ThrowIfFailed(hr);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> mfSampleTexture;
    hr = mfSampleDxgiBuffer->GetResource(IID_PPV_ARGS(&mfSampleTexture));
    H::System::ThrowIfFailed(hr);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> dstTexture;
    {
        H::Dx::MFDXGIDeviceManagerLock mfDxgiDeviceManagerLock{ this->mfDxgiDeviceManager }; // it may block current thread when device lock / unlock

        Microsoft::WRL::ComPtr<ID3D11Device> mfD3dDevice;
        hr = mfDxgiDeviceManagerLock.LockDevice(mfD3dDevice.GetAddressOf());
        //hr = this->mfLock.LockDevice(mfD3dDevice.GetAddressOf());
        if (FAILED(hr)) {
            Dbreak;
        }

        D3D11_TEXTURE2D_DESC srcTextureDesc = {};
        mfSampleTexture->GetDesc(&srcTextureDesc);

        if (!this->sharedTexture) {
            this->sharedTexture = std::make_unique<H::Dx::DxSharedTexture>(srcTextureDesc, this->dxDeviceSafeObj->Lock()->GetD3DDevice(), mfD3dDevice);
        }
        sharedTexture->CopyTexture(dstTexture.GetAddressOf(), mfSampleTexture);

        //hr = this->mfLock.UnlockDevice();
        //if (FAILED(hr)) {
        //    Dbreak;
        //}
    }

    //MF::MFSample mfSampleCopy = *mfSample;
    //mfSampleCopy.pSample = nullptr; // release original reference to IMFSample
    //auto sample = std::make_unique<MF::MFVideoSample>(mfSampleCopy, dstTexture);
    auto sample = std::make_unique<MF::MFVideoSample>(*mfSample, dstTexture);
    return sample;

#else

    auto sample = std::make_unique<MF::MFVideoSample>(*mfSample);
    return sample;
#endif
}