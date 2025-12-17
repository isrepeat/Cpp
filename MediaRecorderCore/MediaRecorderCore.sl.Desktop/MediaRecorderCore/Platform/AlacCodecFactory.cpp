#include "pch.h"
#include "AlacCodecFactory.h"

#include <libhelpers/HSystem.h>
#include <wmcodecdsp.h>

#include <initguid.h>

DEFINE_GUID(CLSID_ALACEncMFT_CopyDef, 0x9AB6A28C, 0x748E, 0x4B6A, 0xBF, 0xFF, 0xCC, 0x44, 0x3B, 0x8E, 0x8F, 0xB4); //IID for MediaFormat_ALAC {9AB6A28C-748E-4B6A-BFFF-CC443B8E8FB4}

Microsoft::WRL::ComPtr<IMFTransform> AlacCodecFactory::CreateIMFTransform() {
	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<IMFTransform> transform;

	hr = CoCreateInstance(CLSID_ALACEncMFT_CopyDef, nullptr, CLSCTX_ALL, IID_PPV_ARGS(transform.GetAddressOf()));
	H::System::ThrowIfFailed(hr);

	return transform;
}