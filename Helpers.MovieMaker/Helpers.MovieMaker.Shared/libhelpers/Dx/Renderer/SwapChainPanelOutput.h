#pragma once
#include "libhelpers/config.h"
#include "IOutput.h"
#include "../Dx.h"
#include "../../raw_ptr.h"

#include "libhelpers/Aligned.h"

#include <Helpers/Dx/SwapChainPanel.h>

#if HAVE_WINRT == 1

class SwapChainPanelOutput : public IOutput {
public:
	SwapChainPanelOutput(Helpers::WinRt::Dx::SwapChainPanel^ swapChainPanelWinRt);
	virtual ~SwapChainPanelOutput();

	Microsoft::WRL::ComPtr<HELPERS_NS::Dx::ISwapChainPanel> GetSwapChainPanelNative();
	Windows::UI::Xaml::Controls::SwapChainPanel^ GetSwapChainPanelXaml() const;

	float GetLogicalDpi() const override;
	DirectX::XMFLOAT2 GetLogicalSize() const override;
	D3D11_VIEWPORT GetD3DViewport() const override;
	ID3D11RenderTargetView *GetD3DRtView() const override;
	ID2D1Bitmap1 *GetD2DRtView() const override;
	D2D1_MATRIX_3X2_F GetD2DOrientationTransform() const override;
	DirectX::XMFLOAT4X4 GetD3DOrientationTransform() const override;
	OrientationTypes GetOrientation() const override;
	OrientationTypes GetNativeOrientation() const override;
	
	void SetRTColor(DirectX::XMFLOAT4 color);
	DirectX::XMFLOAT4 GetRTColor() const;

	Helpers::WinRt::Dx::DxSettings^ GetDxSettings() const;

	void SetLogicalDpi(float v);
	void SetLogicalSize(const DirectX::XMFLOAT2 &v);
	
	void SetCompositionScale(const DirectX::XMFLOAT2 &v);
	DirectX::XMFLOAT2 GetCompositionScale() const;
	
	void SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations v);
	Windows::Graphics::Display::DisplayOrientations GetCurrentOrientation() const;
	
	void Render(std::function<void()> renderHandler);

protected:
	void BeginRender();
	void EndRender();

private:
	std::mutex mx;
	Helpers::WinRt::Dx::SwapChainPanel^ swapChainPanelWinRt;
	Microsoft::WRL::ComPtr<HELPERS_NS::Dx::ISwapChainPanel> swapChainPanelNative;
	DirectX::XMFLOAT4 rtColor;

	//Windows::Foundation::EventRegistrationToken dxSettingsMsaaChangedToken;
	//Windows::Foundation::EventRegistrationToken dxSettingsCurrentAdapterChangedToken;
};
#endif