#include "pch.h"
#include "AacCodecFactory.h"

#include <libhelpers/HSystem.h>
#include <wmcodecdsp.h>

Microsoft::WRL::ComPtr<IMFTransform> AacCodecFactory::CreateIMFTransform() {
	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<IMFTransform> transform;

	hr = CoCreateInstance(__uuidof(AACMFTEncoder), nullptr, CLSCTX_ALL, IID_PPV_ARGS(transform.GetAddressOf()));
	H::System::ThrowIfFailed(hr);

	return transform;
}