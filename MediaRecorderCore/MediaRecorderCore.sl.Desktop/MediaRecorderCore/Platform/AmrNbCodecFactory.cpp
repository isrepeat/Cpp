#include "pch.h"
#include "AmrNbCodecFactory.h"

#include <libhelpers/HSystem.h>
#include <initguid.h>

DEFINE_GUID(AMR_NB_ENCODER, 0x2FAE8AFE, 0x04A3, 0x423a, 0xA8, 0x14, 0x85, 0xDB, 0x45, 0x47, 0x12, 0xB0);

Microsoft::WRL::ComPtr<IMFTransform> AmrNbCodecFactory::CreateIMFTransform() {
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFTransform> transform;

    hr = CoCreateInstance(AMR_NB_ENCODER, nullptr, CLSCTX_ALL, IID_PPV_ARGS(transform.GetAddressOf()));
    H::System::ThrowIfFailed(hr);

    return transform;
}