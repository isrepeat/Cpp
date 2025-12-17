#include "pch.h"
#include "SwapChainPanelOutput.h"
#include "../../HSystem.h"
#include "../../WinRT/RunOnUIThread.h"

#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <windows.ui.xaml.media.dxinterop.h>

#if HAVE_WINRT == 1

namespace Tools {
	OrientationTypes DisplayOrientationFromNative(HELPERS_NS::Dx::DisplayOrientations displayOrientation) {
		switch (displayOrientation) {
		case HELPERS_NS::Dx::DisplayOrientations::Landscape:
			return OrientationTypes::Landscape;

		case HELPERS_NS::Dx::DisplayOrientations::Portrait:
			return OrientationTypes::Portrait;

		case HELPERS_NS::Dx::DisplayOrientations::LandscapeFlipped:
			return OrientationTypes::FlippedLandscape;

		case HELPERS_NS::Dx::DisplayOrientations::PortraitFlipped:
			return OrientationTypes::FlippedPortrait;

		default:
			return OrientationTypes::None;
		}
	}
}

SwapChainPanelOutput::SwapChainPanelOutput(Helpers::WinRt::Dx::SwapChainPanel^ swapChainPanelWinRt)
	: swapChainPanelWinRt{ swapChainPanelWinRt }
	, swapChainPanelNative{ HELPERS_NS::Dx::WinRt::Tools::QuerySwapChainPanelNative(this->swapChainPanelWinRt->GetSwapChainPanelNativeAsObject()) }
{
	{
		auto tmp = DirectX::Colors::LightGreen;
		this->rtColor = { tmp.f[0], tmp.f[1], tmp.f[2], tmp.f[3] };
	}

	// TODO: create dxSettings in swapChainPanelWinRt and suscribe here

	//// TODO: add guards for destroyed 'this'
	//this->dxSettingsMsaaChangedToken = this->dxSettings->MsaaChanged += ref new Helpers::WinRt::EventHandler([this] {
	//	this->CreateWindowSizeDependentResources();
	//});
	//this->dxSettingsCurrentAdapterChangedToken = this->dxSettings->CurrentAdapterChanged += ref new Helpers::WinRt::EventHandler([this] {
	//	// recreate dxDev ...
	//	});
}

SwapChainPanelOutput::~SwapChainPanelOutput() {
	//this->dxSettings->MsaaChanged -= this->dxSettingsMsaaChangedToken;
	//this->dxSettings->CurrentAdapterChanged -= this->dxSettingsCurrentAdapterChangedToken;
}

Microsoft::WRL::ComPtr<HELPERS_NS::Dx::ISwapChainPanel> SwapChainPanelOutput::GetSwapChainPanelNative() {
	return this->swapChainPanelNative;
}

Windows::UI::Xaml::Controls::SwapChainPanel^ SwapChainPanelOutput::GetSwapChainPanelXaml() const {
	return this->swapChainPanelWinRt->GetSwapChainPanelXaml();
}

float SwapChainPanelOutput::GetLogicalDpi() const {
	return this->swapChainPanelNative->GetDpi();
}

DirectX::XMFLOAT2 SwapChainPanelOutput::GetLogicalSize() const {
	auto size = this->swapChainPanelNative->GetLogicalSize();
	return { size.width, size.height };
}

D3D11_VIEWPORT SwapChainPanelOutput::GetD3DViewport() const {
	return this->swapChainPanelNative->GetScreenViewport();
}

ID3D11RenderTargetView *SwapChainPanelOutput::GetD3DRtView() const {
	return this->swapChainPanelNative->GetRenderTargetView().Get();
}

ID2D1Bitmap1 *SwapChainPanelOutput::GetD2DRtView() const {
	return this->swapChainPanelNative->GetD2DTargetBitmap().Get();
}

D2D1_MATRIX_3X2_F SwapChainPanelOutput::GetD2DOrientationTransform() const {
	return this->swapChainPanelNative->GetOrientationTransform2D();
}

DirectX::XMFLOAT4X4 SwapChainPanelOutput::GetD3DOrientationTransform() const {
	return this->swapChainPanelNative->GetOrientationTransform3D();
}

OrientationTypes SwapChainPanelOutput::GetOrientation() const {
	return Tools::DisplayOrientationFromNative(this->swapChainPanelNative->GetCurrentOrientation());
}

OrientationTypes SwapChainPanelOutput::GetNativeOrientation() const {
	return Tools::DisplayOrientationFromNative(this->swapChainPanelNative->GetNativeOrientation());
}


void SwapChainPanelOutput::SetRTColor(DirectX::XMFLOAT4 color) {
	this->rtColor = color;
}

DirectX::XMFLOAT4 SwapChainPanelOutput::GetRTColor() const {
	return this->rtColor;
}

Helpers::WinRt::Dx::DxSettings^ SwapChainPanelOutput::GetDxSettings() const {
	return this->swapChainPanelWinRt->GetDxSettings();
}

void SwapChainPanelOutput::SetLogicalDpi(float v) {
	this->swapChainPanelNative->SetDpi(v);
}

void SwapChainPanelOutput::SetLogicalSize(const DirectX::XMFLOAT2 &v) {
	this->swapChainPanelNative->SetLogicalSize({ v.x, v.y });
}

void SwapChainPanelOutput::SetCompositionScale(const DirectX::XMFLOAT2 &v) {
	this->swapChainPanelNative->SetCompositionScale(v.x, v.y);
}

DirectX::XMFLOAT2 SwapChainPanelOutput::GetCompositionScale() const {
	return this->swapChainPanelNative->GetCompositionScale();
}

Windows::Graphics::Display::DisplayOrientations SwapChainPanelOutput::GetCurrentOrientation() const {
	return this->swapChainPanelWinRt->GetCurrentOrientation();
}

void SwapChainPanelOutput::SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations v) {
	return this->swapChainPanelWinRt->SetCurrentOrientation(v);
}

void SwapChainPanelOutput::Render(std::function<void()> renderHandler) {
	auto tp1 = std::chrono::high_resolution_clock::now();

	// save to tmp because while unlocked m_frameLatencyWaitableObject can change in resize
	//auto tmpFrameLatencyWaitableObject = this->frameLatencyWaitableObject;
	//DWORD result = WaitForSingleObjectEx(
	//	tmpFrameLatencyWaitableObject,
	//	1000, // 1 second timeout (shouldn't ever occur)
	//	true
	//);

	auto tp2 = std::chrono::high_resolution_clock::now();

	//LOG_DEBUG_D("\n"
	//	"dt [Vsync] = {}ms\n"
	//	, HH::Chrono::milliseconds_f{ tp2 - tp1 }.count()
	//);


	auto tp3 = std::chrono::high_resolution_clock::now();
	std::lock_guard lk{ mx };

	auto dxDeviceSafeObj = this->swapChainPanelNative->GetDxDevice();
	auto dxDev = dxDeviceSafeObj->Lock();

	auto tp4 = std::chrono::high_resolution_clock::now();
	
	this->BeginRender();
	
	auto tp5 = std::chrono::high_resolution_clock::now();
	
	if (renderHandler) {
		renderHandler();
	}
	
	auto tp6 = std::chrono::high_resolution_clock::now();
	
	this->EndRender();
	
	auto tp7 = std::chrono::high_resolution_clock::now();
	//LOG_DEBUG_D("\n"
	//	"dt [Vsync] = {}ms\n"
	//	"dt [BeginRender] = {}ms\n"
	//	"dt [Render] = {}ms\n"
	//	"dt [EndRender] = {}ms\n"
	//	"SwapChainRender iteration = {}ms\n"
	//	, HH::Chrono::milliseconds_f{ tp2 - tp1 }.count()
	//	, HH::Chrono::milliseconds_f{ tp5 - tp4 }.count()
	//	, HH::Chrono::milliseconds_f{ tp6 - tp5 }.count()
	//	, HH::Chrono::milliseconds_f{ tp7 - tp6 }.count()
	//	, HH::Chrono::milliseconds_f{ tp7 - tp1 }.count()
	//);
}

void SwapChainPanelOutput::BeginRender() {
	auto dxDeviceSafeObj = this->swapChainPanelNative->GetDxDevice();
	auto dxDev = dxDeviceSafeObj->Lock();

	ID3D11RenderTargetView *const targets[1] = { this->GetD3DRtView() };
	auto dxCtx = dxDev->LockContext();

	dxCtx->D3D()->OMSetRenderTargets(1, targets, nullptr);
	DirectX::XMVECTORF32 tmp = { 
        this->rtColor.x * this->rtColor.w,
		this->rtColor.y * this->rtColor.w,
		this->rtColor.z * this->rtColor.w,
		this->rtColor.w };
	dxCtx->D3D()->ClearRenderTargetView(this->GetD3DRtView(), tmp);
}

void SwapChainPanelOutput::EndRender() {
	this->swapChainPanelNative->Present();
}
#endif