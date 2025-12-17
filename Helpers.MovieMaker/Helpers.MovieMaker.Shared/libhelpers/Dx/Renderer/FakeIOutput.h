#pragma once
#include "IOutput.h"
#include "../Dx.h"
#include "../../raw_ptr.h"

class FakeIOutput : public IOutput {
    static const uint32_t RtWidth = 32;
    static const uint32_t RtHeight = 16;
public:
    FakeIOutput(raw_ptr<DxDevice> dxDev);
    virtual ~FakeIOutput();

    float GetLogicalDpi() const override;
    DirectX::XMFLOAT2 GetLogicalSize() const override;
    D3D11_VIEWPORT GetD3DViewport() const override;
    ID3D11RenderTargetView* GetD3DRtView() const override;
    ID2D1Bitmap1* GetD2DRtView() const override;
    D2D1_MATRIX_3X2_F GetD2DOrientationTransform() const override;
    DirectX::XMFLOAT4X4 GetD3DOrientationTransform() const override;

    OrientationTypes GetOrientation() const override;
    OrientationTypes GetNativeOrientation() const override;

    // Just sets rendertargets' to context
    void BeginRender();

private:
    raw_ptr<DxDevice> dxDeviceSafeObj;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3dRtView;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dBitmap;
};