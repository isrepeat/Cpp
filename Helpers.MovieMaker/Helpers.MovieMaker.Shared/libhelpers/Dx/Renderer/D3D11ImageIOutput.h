#pragma once
#include "IOutput.h"
#include "../../raw_ptr.h"
#include "../../Structs.h"

class D3D11ImageIOutput : public IOutput {
public:
	D3D11ImageIOutput(raw_ptr<DxDevice> dxDeviceSafeObj);
	virtual ~D3D11ImageIOutput();

	float GetLogicalDpi() const override;
	DirectX::XMFLOAT2 GetLogicalSize() const override;
	D3D11_VIEWPORT GetD3DViewport() const override;
	ID3D11RenderTargetView* GetD3DRtView() const override;
	ID2D1Bitmap1* GetD2DRtView() const override;
	D2D1_MATRIX_3X2_F GetD2DOrientationTransform() const override;
	DirectX::XMFLOAT4X4 GetD3DOrientationTransform() const override;

	void ResetRenderTarget(void* surface);

	bool BeginRender();
	void EndRender();

	OrientationTypes GetOrientation() const override;
	OrientationTypes GetNativeOrientation() const override;

	Structs::RgbaF GetRTColor();
	void SetRTColor(const Structs::RgbaF& color);

private:
	raw_ptr<DxDevice> dxDeviceSafeObj;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3dRenderTargetView;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dTargetBitmap;
	D2D1_MATRIX_3X2_F d2dOrientationTransform;
	DirectX::XMFLOAT4X4 d3dOrientationTransform;

	float logicalDpi;
	DirectX::XMFLOAT2 logicalSize;

	Structs::RgbaF rtColor;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> GetOutputResource(void* surface);
};