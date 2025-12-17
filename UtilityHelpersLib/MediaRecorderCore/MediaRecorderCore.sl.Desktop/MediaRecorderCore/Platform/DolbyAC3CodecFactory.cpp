#include "pch.h"
#include "DolbyAC3CodecFactory.h"

#include <libhelpers/HSystem.h>
#include <wmcodecdsp.h>

Microsoft::WRL::ComPtr<IMFTransform> DolbyAC3CodecFactory::CreateIMFTransform() {
	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<IMFTransform> transform;

	hr = CoCreateInstance(__uuidof(CMSDolbyDigitalEncMFT), nullptr, CLSCTX_ALL, IID_PPV_ARGS(transform.GetAddressOf()));
	H::System::ThrowIfFailed(hr);

	return transform;
}