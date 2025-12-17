#include "pch.h"
#include "D3D9Dev.h"

#if HAVE_WINRT == 0
#include "..\HSystem.h"

D3D9Dev::D3D9Dev() {
    HRESULT hr = S_OK;

    hr = Direct3DCreate9Ex(D3D_SDK_VERSION, this->d3d9.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    D3DPRESENT_PARAMETERS d3dpp = {};

    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = NULL;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    this->dummyHWND = CreateWindowA("STATIC", "dummy", NULL, 0, 0, 100, 100, NULL, NULL, NULL, NULL);

    hr = this->d3d9->CreateDeviceEx(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        this->dummyHWND,
        D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
        &d3dpp,
        NULL,
        this->d3d9Dev.GetAddressOf());
    H::System::ThrowIfFailed(hr);
}

D3D9Dev::~D3D9Dev() {
    DestroyWindow(this->dummyHWND);
}

IDirect3D9Ex *D3D9Dev::GetD3D() {
    return this->d3d9.Get();
}

IDirect3DDevice9Ex *D3D9Dev::GetD3DDev() {
    return this->d3d9Dev.Get();
}
#endif // HAVE_WINRT == 0
