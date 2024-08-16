#include "DxDevice.h"
#include <dxgi1_3.h>
#include <combaseapi.h>
#include <Helpers/System.h>
#include <Helpers/Macros.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxguid.lib")

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))


DxDevice::DxDevice()
    : featureLevel(D3D_FEATURE_LEVEL_11_0)
{
    this->CreateDeviceIndependentResources();
    this->CreateDeviceDependentResources();
}

DxDevice::~DxDevice() {
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDev;

    hr = this->d3dDevice.As(&dxgiDev);

    if (SUCCEEDED(hr)) {
        // https://msdn.microsoft.com/en-us/library/windows/desktop/dn280346(v=vs.85).aspx
        // apps should call ID3D11DeviceContext::ClearState before calling Trim
        {
            this->ctx->D3D()->ClearState();
        }

        dxgiDev->Trim();
    }
}

std::shared_ptr<DxDeviceCtx> DxDevice::GetContext() const {
    return ctx;
}

D3D_FEATURE_LEVEL DxDevice::GetDeviceFeatureLevel() const {
    return this->featureLevel;
}

Microsoft::WRL::ComPtr<IDWriteFactory> DxDevice::GetDwriteFactory() const {
    return dwriteFactory;
}

Microsoft::WRL::ComPtr<ID2D1Factory1> DxDevice::GetD2DFactory() const {
    return d2dFactory;
}

Microsoft::WRL::ComPtr<ID3D11Device> DxDevice::GetD3DDevice() const {
    return d3dDevice;
}

Microsoft::WRL::ComPtr<ID2D1Device> DxDevice::GetD2DDevice() const {
    return d2dDevice;
}

Microsoft::WRL::ComPtr<IDXGISwapChain1> DxDevice::CreateSwapChainForHwnd(DXGI_SWAP_CHAIN_DESC1 swapChainDesc, HWND hwnd) {
    HRESULT hr = S_OK;

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    H::System::ThrowIfFailed(this->d3dDevice.As(&dxgiDevice));

    Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
    H::System::ThrowIfFailed(dxgiDevice->GetAdapter(&dxgiAdapter));

    Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
    H::System::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
    hr = dxgiFactory->CreateSwapChainForHwnd(d3dDevice.Get(), hwnd, &swapChainDesc, nullptr, nullptr, swapChain.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    return swapChain;
}


void DxDevice::CreateDeviceIndependentResources() {
    HRESULT hr = S_OK;

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        (IUnknown**)this->dwriteFactory.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, IID_PPV_ARGS(this->d2dFactory.GetAddressOf()));
    H::System::ThrowIfFailed(hr);
}

void DxDevice::CreateDeviceDependentResources() {
    HRESULT hr = S_OK;
    uint32_t flags;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dCtxTmp;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3dCtx;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dCtx;

    flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, flags,
        featureLevels, ARRAY_SIZE(featureLevels),
        D3D11_SDK_VERSION,
        this->d3dDevice.GetAddressOf(), &this->featureLevel,
        d3dCtxTmp.GetAddressOf());

    if (FAILED(hr)) {
        hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP,
            nullptr, flags,
            featureLevels, ARRAY_SIZE(featureLevels),
            D3D11_SDK_VERSION,
            this->d3dDevice.GetAddressOf(), &this->featureLevel,
            d3dCtxTmp.GetAddressOf());
        H::System::ThrowIfFailed(hr);
    }

    hr = d3dCtxTmp.As(&d3dCtx);
    H::System::ThrowIfFailed(hr);

    this->CreateD2DDevice();
    d2dCtx = this->CreateD2DDeviceContext();

    this->ctx = std::make_shared<DxDeviceCtx>(d3dCtx, d2dCtx);
}


void DxDevice::CreateD2DDevice() {
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDev;

    hr = this->d3dDevice.As(&dxgiDev);
    H::System::ThrowIfFailed(hr);

    hr = this->d2dFactory->CreateDevice(dxgiDev.Get(), this->d2dDevice.GetAddressOf());
    H::System::ThrowIfFailed(hr);
}

Microsoft::WRL::ComPtr<ID2D1DeviceContext> DxDevice::CreateD2DDeviceContext() {
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dCtx;

    hr = this->d2dDevice->CreateDeviceContext(
        D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
        d2dCtx.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    return d2dCtx;
}
