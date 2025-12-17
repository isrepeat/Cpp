#include "pch.h"
#include "FakeIOutput.h"

#include "..\DxHelpers.h"
#include "..\..\HMath.h"
#include "..\..\HSystem.h"

FakeIOutput::FakeIOutput(raw_ptr<DxDevice> dxDeviceSafeObj)
    : dxDeviceSafeObj(dxDeviceSafeObj)
{
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2d;
    Microsoft::WRL::ComPtr<IDXGISurface> dxgiBackBuffer;
    auto dxDev = this->dxDeviceSafeObj->Lock();

    auto d3dDev = dxDev->GetD3DDevice();
    auto d2dCtxMt = dxDev->GetD2DCtxMt();

    D3D11_TEXTURE2D_DESC tex2dDesc;
    D2D1_BITMAP_PROPERTIES1 bitmapProperties =
        D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            DxHelpers::standartDpi.x,
            DxHelpers::standartDpi.y
        );

    // small texture for fake output
    tex2dDesc.Width = FakeIOutput::RtWidth;
    tex2dDesc.Height = FakeIOutput::RtHeight;
    tex2dDesc.MipLevels = 1;
    tex2dDesc.ArraySize = 1;
    tex2dDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    tex2dDesc.SampleDesc.Count = 1;
    tex2dDesc.SampleDesc.Quality = 0;
    tex2dDesc.Usage = D3D11_USAGE_DEFAULT;
    tex2dDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    tex2dDesc.CPUAccessFlags = 0;
    tex2dDesc.MiscFlags = 0;

    hr = d3dDev->CreateTexture2D(&tex2dDesc, nullptr, tex2d.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = tex2d.As(&dxgiBackBuffer);
    H::System::ThrowIfFailed(hr);

    hr = d3dDev->CreateRenderTargetView(tex2d.Get(), nullptr, this->d3dRtView.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = d2dCtxMt->CreateBitmapFromDxgiSurface(dxgiBackBuffer.Get(), &bitmapProperties, this->d2dBitmap.GetAddressOf());
    H::System::ThrowIfFailed(hr);
}

FakeIOutput::~FakeIOutput() {
}

float FakeIOutput::GetLogicalDpi() const {
    return DxHelpers::standartDpi.x;
}

DirectX::XMFLOAT2 FakeIOutput::GetLogicalSize() const {
    DirectX::XMFLOAT2 size(
        (float)FakeIOutput::RtWidth,
        (float)FakeIOutput::RtHeight);
    return size;
}

D3D11_VIEWPORT FakeIOutput::GetD3DViewport() const {
    D3D11_VIEWPORT viewport;

    viewport.TopLeftX = viewport.TopLeftY = 0;
    viewport.Width = (float)FakeIOutput::RtWidth;
    viewport.Height = (float)FakeIOutput::RtHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    return viewport;
}

ID3D11RenderTargetView* FakeIOutput::GetD3DRtView() const {
    return this->d3dRtView.Get();
}

ID2D1Bitmap1* FakeIOutput::GetD2DRtView() const {
    return this->d2dBitmap.Get();
}

D2D1_MATRIX_3X2_F FakeIOutput::GetD2DOrientationTransform() const {
    return D2D1::IdentityMatrix();
}

DirectX::XMFLOAT4X4 FakeIOutput::GetD3DOrientationTransform() const {
    return H::Math::Identity<DirectX::XMFLOAT4X4>();
}

OrientationTypes FakeIOutput::GetOrientation() const {
    return OrientationTypes::Landscape;
}

OrientationTypes FakeIOutput::GetNativeOrientation() const {
    return OrientationTypes::Landscape;
}

void FakeIOutput::BeginRender() {
    auto rtView = this->GetD3DRtView();
    ID3D11RenderTargetView* const targets[1] = { this->GetD3DRtView() };
    auto dxDev = this->dxDeviceSafeObj->Lock();
    auto dxCtx = dxDev->LockContext();
    auto viewport = this->GetD3DViewport();

    dxCtx->D3D()->OMSetRenderTargets(1, targets, nullptr);
    dxCtx->D3D()->RSSetViewports(1, &viewport);
    dxCtx->D3D()->ClearRenderTargetView(this->GetD3DRtView(), DirectX::Colors::Transparent);

    dxCtx->D2D()->SetTarget(this->d2dBitmap.Get());
    dxCtx->D2D()->SetDpi(DxHelpers::standartDpi.x, DxHelpers::standartDpi.y);
    dxCtx->D2D()->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}