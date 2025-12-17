#include "pch.h"
#include "MP3CodecFactory.h"

#include <libhelpers/HSystem.h>
#include <wmcodecdsp.h>

Microsoft::WRL::ComPtr<IMFTransform> MP3CodecFactory::CreateIMFTransform() {
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFTransform> transform;

    hr = CoCreateInstance(__uuidof(MP3ACMCodecWrapper), nullptr, CLSCTX_ALL, IID_PPV_ARGS(transform.GetAddressOf()));
    H::System::ThrowIfFailed(hr);

    return transform;
}