#pragma once
#include "..\config.h"

#if HAVE_WINRT == 0
#include <d3d9.h>
#include <wrl.h>

#pragma comment(lib, "D3d9.lib")
#pragma comment(lib, "User32.lib")

class D3D9Dev {
public:
    D3D9Dev();
    ~D3D9Dev();

    IDirect3D9Ex *GetD3D();
    IDirect3DDevice9Ex *GetD3DDev();

private:
    Microsoft::WRL::ComPtr<IDirect3D9Ex> d3d9;
    Microsoft::WRL::ComPtr<IDirect3DDevice9Ex> d3d9Dev;
    HWND dummyHWND;
};
#endif // HAVE_WINRT == 0