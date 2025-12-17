#include "pch.h"
#include "Wma8CodecFactory.h"

#include <libhelpers/HSystem.h>
#include <wmcodecdsp.h>

Microsoft::WRL::ComPtr<IMFTransform> Wma8CodecFactory::CreateIMFTransform() {
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFTransform> transform;

    hr = CoCreateInstance(__uuidof(CWMAEncMediaObject), nullptr, CLSCTX_ALL, IID_PPV_ARGS(transform.GetAddressOf()));
    H::System::ThrowIfFailed(hr);

    return transform;
}