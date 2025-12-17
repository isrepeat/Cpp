#include "AvDxgiBufferFactory.h"

#include <libhelpers\HSystem.h>

AvDxgiBufferFactory::AvDxgiBufferFactory(
    DxDevice* dxDeviceSafeObj,
    MFCreateDXGISurfaceBufferPtr createSurfBuffer,
    MFCreateDXGIDeviceManagerPtr createDxMan)
    : dxDeviceSafeObj(dxDeviceSafeObj)
    , createSurfBuffer(createSurfBuffer)
    , createDxMan(createDxMan)
{}

Microsoft::WRL::ComPtr<IMFMediaBuffer> AvDxgiBufferFactory::CreateBuffer(ID3D11DeviceContext*/*d3dCtx*/, ID3D11Texture2D* tex) {
    HRESULT hr = S_OK;
    D3D11_TEXTURE2D_DESC texDesc;
    Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer;

    tex->GetDesc(&texDesc);

    DWORD bufferByteSize = texDesc.Width * texDesc.Height * 4;

    hr = this->createSurfBuffer(__uuidof(tex), tex, 0, FALSE, buffer.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    /*
    https://msdn.microsoft.com/en-us/library/windows/desktop/hh162751%28v=vs.85%29.aspx
    Setting the buffer length
    If you plan to add this buffer to an IMFSample, and pass it to an IMFSinkWriter instance, and you're left scratching your head
    because IMFSinkWriter::WriteSample is returning E_INVALIDARG then the problem is probably that you need to set the buffer length.
    To do this, query the buffer for the IMF2DBuffer interface, call GetContinuousLength to get the buffer length,
    then on the IMFMediaBuffer interface call SetCurrentLength with that value.
    */
    /*DWORD bufferByteSize2, bufferByteSize3;
    buffer->GetCurrentLength(&bufferByteSize2);

    Microsoft::WRL::ComPtr<IMF2DBuffer> tmp;
    buffer.As(&tmp);
    tmp->GetContiguousLength(&bufferByteSize3);*/

    hr = buffer->SetCurrentLength(bufferByteSize);
    H::System::ThrowIfFailed(hr);

    return buffer;
}

void AvDxgiBufferFactory::SetAttributes(IMFAttributes* attr) {
    HRESULT hr = S_OK;

    if (!this->mfDxgiDeviceManager) {
        uint32_t resetToken = 0;

        hr = this->createDxMan(&resetToken, &this->mfDxgiDeviceManager);
        H::System::ThrowIfFailed(hr);

        hr = this->mfDxgiDeviceManager->ResetDevice(this->dxDeviceSafeObj->Lock()->GetD3DDevice().Get(), resetToken);
        H::System::ThrowIfFailed(hr);
    }

    hr = attr->SetUnknown(MF_SINK_WRITER_D3D_MANAGER, this->mfDxgiDeviceManager.Get());
    H::System::ThrowIfFailed(hr);
}
