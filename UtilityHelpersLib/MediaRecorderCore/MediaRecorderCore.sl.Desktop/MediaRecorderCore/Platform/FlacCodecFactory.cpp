#include "pch.h"
#include "FlacCodecFactory.h"

#include <libhelpers/HSystem.h>
#include <wmcodecdsp.h>

#include <initguid.h>

DEFINE_GUID(CLSID_FLACEncMFT_CopyDef, 0x128509E9, 0xC44E, 0x45DC, 0x95, 0xE9, 0xC2, 0x55, 0xB8, 0xF4, 0x66, 0xA6); //IID for MediaFormat_FLAC {128509E9-C44E-45DC-95E9-C255B8F466A6}

Microsoft::WRL::ComPtr<IMFTransform> FlacCodecFactory::CreateIMFTransform() {
	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<IMFTransform> transform;

	hr = CoCreateInstance(CLSID_FLACEncMFT_CopyDef, nullptr, CLSCTX_ALL, IID_PPV_ARGS(transform.GetAddressOf()));
	H::System::ThrowIfFailed(hr);

	return transform;
}