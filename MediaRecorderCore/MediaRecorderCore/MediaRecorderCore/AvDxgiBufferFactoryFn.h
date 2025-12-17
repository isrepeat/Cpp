#pragma once

#include <Windows.h>
#include <libhelpers\MediaFoundation\MFUser.h>

typedef HRESULT(STDAPICALLTYPE *MFCreateDXGISurfaceBufferPtr)(
    _In_ REFIID riid,
    _In_ IUnknown* punkSurface,
    _In_ UINT uSubresourceIndex,
    _In_ BOOL fBottomUpWhenLinear,
    _Outptr_ IMFMediaBuffer** ppBuffer
    );

// MFCreateDXGIDeviceManager used as pointer because it's not available on Win7
// but library will check it in runtime
typedef HRESULT(STDAPICALLTYPE
    *MFCreateDXGIDeviceManagerPtr)(
        _Out_ UINT* resetToken,
        _Outptr_ IMFDXGIDeviceManager** ppDeviceManager
        );