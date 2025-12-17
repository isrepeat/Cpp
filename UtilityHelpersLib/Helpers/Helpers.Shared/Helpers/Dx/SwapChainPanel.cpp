#include "SwapChainPanel.h"
#include <MagicEnum/MagicEnum.h>
#include <Helpers/Dx/DxHelpers.h>
#include <Helpers/Passkey.hpp>
#include <Helpers/Logger.h>
#include <Helpers/System.h>

#include <windows.ui.xaml.media.dxinterop.h>

namespace DisplayMetrics {
	// High resolution displays can require a lot of GPU and battery power to render.
	// High resolution phones, for example, may suffer from poor battery life if
	// games attempt to render at 60 frames per second at full fidelity.
	// The decision to render at full fidelity across all platforms and form factors
	// should be deliberate.
	static const bool SupportHighResolutions = false;

	// The default thresholds that define a "high resolution" display. If the thresholds
	// are exceeded and SupportHighResolutions is false, the dimensions will be scaled
	// by 50%.
	static const float DpiThreshold = 192.0f;		// 200% of standard desktop display.
	static const float WidthThreshold = 1920.0f;	// 1080p width.
	static const float HeightThreshold = 1080.0f;	// 1080p height.
};

// Constants used to calculate screen rotations.
namespace ScreenRotation {
	// 0-degree Z-rotation
	static const DirectX::XMFLOAT4X4 Rotation0(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// 90-degree Z-rotation
	static const DirectX::XMFLOAT4X4 Rotation90(
		0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// 180-degree Z-rotation
	static const DirectX::XMFLOAT4X4 Rotation180(
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// 270-degree Z-rotation
	static const DirectX::XMFLOAT4X4 Rotation270(
		0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
};

namespace Tools {
	inline DXGI_FORMAT WithoutSRGB(DXGI_FORMAT fmt) noexcept {
		switch (fmt) {
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM;
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8X8_UNORM;
		default: return fmt;
		}
	}

	inline DXGI_SWAP_EFFECT WithFlip(DXGI_SWAP_EFFECT effect) noexcept {
		switch (effect) {
		case DXGI_SWAP_EFFECT_DISCARD: return DXGI_SWAP_EFFECT_FLIP_DISCARD;
		case DXGI_SWAP_EFFECT_SEQUENTIAL: return DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		default: return effect;
		}
	}

	inline long ComputeIntersectionArea(
		long ax1, long ay1, long ax2, long ay2,
		long bx1, long by1, long bx2, long by2) noexcept
	{
		return std::max(0l, std::min(ax2, bx2) - std::max(ax1, bx1)) * std::max(0l, std::min(ay2, by2) - std::max(ay1, by1));
	}
}

namespace HELPERS_NS {
	namespace Dx {
		SwapChainPanel::SwapChainPanel(const InitData& initData)
			: initData{ initData }
			, dxDeviceSafeObj{ initData.dxDeviceSafeObjMutexFactory(), initData.dxDeviceFactory() }
			, m_screenViewport{}
			, m_d3dRenderTargetSize{}
			, m_outputSize{}
			, m_logicalSize{}
			, m_nativeOrientation{ DXGI_MODE_ROTATION::DXGI_MODE_ROTATION_IDENTITY }
			, m_currentOrientation{ DXGI_MODE_ROTATION::DXGI_MODE_ROTATION_IDENTITY }
			, m_dpi{ -1.0f }
			, m_effectiveDpi{ -1.0f }
			, m_compositionScaleX{ 1.0f }
			, m_compositionScaleY{ 1.0f }
			, m_resolutionScale{ 1.0f }
			, colorSpace{ DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 }
			, isDisplayHDR10{ false }
			, deviceNotify{ nullptr }
		{
			HRESULT hr = S_OK;
			{
				auto dxDev = this->dxDeviceSafeObj.Lock();
				dxDev->SetAssociatedSwapChainPanel(Passkey<SwapChainPanel*>{}, this);
			}

			// Disable HDR if we are on an OS that can't support FLIP swap effects.
			if (this->initData.optionFlags.Has(InitData::Options::EnableHDR)) {
				auto dxDev = this->dxDeviceSafeObj.Lock();
				auto d3dDevice = dxDev->GetD3DDevice();
				auto dxgiFactory = dxDev->GetDxgiFactory();

				Microsoft::WRL::ComPtr<IDXGIFactory5> dxgiFactory5;
				if (FAILED(dxgiFactory.As(&dxgiFactory5))) {
					const_cast<InitData&>(this->initData).optionFlags &= ~InitData::Options::EnableHDR;
					LOG_WARNING_D("HDR swap chains not supported, disable 'InitData::Options::EnableHDR'");
				}
			}

			// Set dxSettings handlers.
			if (auto dxSettings = this->initData.dxSettingsWeak.lock()) {
				dxSettings->GetEvents()->msaaChanged.Subscribe([this] {
					this->CreateWindowSizeDependentResources();
				});

				dxSettings->GetEvents()->vsyncChanged.Subscribe([] {
				});
			}
		}

		SwapChainPanel::~SwapChainPanel() {
		}

		H::Dx::DxDeviceSafeObj* STDMETHODCALLTYPE SwapChainPanel::GetDxDevice() {
			return &this->dxDeviceSafeObj;
		}

		void STDMETHODCALLTYPE SwapChainPanel::InitSwapChainPanelInfo(
			H::Size_f logicalSize,
			DisplayOrientations nativeOrientation,
			DisplayOrientations currentOrientation,
			float compositionScaleX,
			float compositionScaleY,
			float dpi
		)
		{
			auto dxDev = this->dxDeviceSafeObj.Lock();
			auto dxCtx = dxDev->LockContext();
			auto d2dCtx = dxCtx->D2D();

			this->m_logicalSize = logicalSize;
			this->m_nativeOrientation = nativeOrientation;
			this->m_currentOrientation = currentOrientation;
			this->m_compositionScaleX = compositionScaleX;
			this->m_compositionScaleY = compositionScaleY;
			this->m_dpi = dpi;

			d2dCtx->SetDpi(this->m_dpi, this->m_dpi);
			this->CreateWindowSizeDependentResources();
		}

		// These resources need to be recreated every time the window size is changed.
		void SwapChainPanel::CreateWindowSizeDependentResources() {
			HRESULT hr = S_OK;

			auto dxDev = this->dxDeviceSafeObj.Lock();
			auto d3dDevice = dxDev->GetD3DDevice();
			auto dxCtx = dxDev->LockContext();
			auto d2dCtx = dxCtx->D2D();
			auto d3dCtx = dxCtx->D3D();

			// Clear the previous window size specific context.
			ID3D11RenderTargetView* nullViews[] = { nullptr };
			d3dCtx->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
			d2dCtx->SetTarget(nullptr);
			this->dxgiSwapChainBackBuffer.Reset();
			this->m_d3dRenderTargetView.Reset();
			this->m_d3dDepthStencilView.Reset();
			//this->msaaTexture.Reset();
			//this->msaaRenderTargetView.Reset();

			this->m_d2dTargetBitmap.Reset();
			d3dCtx->Flush1(D3D11_CONTEXT_TYPE_ALL, nullptr);

			this->UpdateRenderTargetSize();

			// The width and height of the swap chain must be based on the window's
			// natively-oriented width and height. If the window is not in the native
			// orientation, the dimensions must be reversed.
			DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

			bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
			this->m_d3dRenderTargetSize.width = swapDimensions ? this->m_outputSize.height : this->m_outputSize.width;
			this->m_d3dRenderTargetSize.height = swapDimensions ? this->m_outputSize.width : this->m_outputSize.height;
			
			const DXGI_FORMAT backBufferFormat = (this->initData.optionFlags & (InitData::Options::EnableHDR)) 
				? ::Tools::WithoutSRGB(this->initData.backBufferFormat)
				: this->initData.backBufferFormat;

			if (this->dxgiSwapChain) {
				// If the swap chain already exists, resize it.
				HRESULT hr = this->dxgiSwapChain->ResizeBuffers(
					2, // Double-buffered swap chain.
					lround(this->m_d3dRenderTargetSize.width),
					lround(this->m_d3dRenderTargetSize.height),
					backBufferFormat,
					0 // or use DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT but create swapChain also with it
				);

				if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
					// If the device was removed for any reason, a new device and swap chain will need to be created.
					this->HandleDeviceLost();

					// Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
					// and correctly set up the new device.
					return;
				}
				else {
					H::System::ThrowIfFailed(hr);
				}
			}
			else {
				// Otherwise, create a new one using the same adapter as the existing Direct3D device.
				//DXGI_SCALING scaling = DisplayMetrics::SupportHighResolutions ? DXGI_SCALING_NONE : DXGI_SCALING_STRETCH;
				DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

				swapChainDesc.Width = lround(this->m_d3dRenderTargetSize.width);		// Match the size of the window.
				swapChainDesc.Height = lround(this->m_d3dRenderTargetSize.height);
				swapChainDesc.Format = backBufferFormat;						// This is the most common swap chain format.
				swapChainDesc.Stereo = false;
				swapChainDesc.SampleDesc.Count = 1;								// Don't use multi-sampling.
				swapChainDesc.SampleDesc.Quality = 0;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.BufferCount = 2;									// Use double-buffering to minimize latency.
				swapChainDesc.SwapEffect = (this->initData.optionFlags.Has(InitData::Options::EnableHDR))  // All Microsoft Store apps must use _FLIP_ SwapEffects.
					? ::Tools::WithFlip(this->initData.dxgiSwapEffect)
					: this->initData.dxgiSwapEffect;
				swapChainDesc.Flags = 0;
				swapChainDesc.Scaling = DXGI_SCALING_STRETCH;					// CreateSwapChainForComposition support only DXGI_SCALING_STRETCH
				swapChainDesc.AlphaMode = this->initData.environment == InitData::Environment::UWP
					? DXGI_ALPHA_MODE_PREMULTIPLIED
					: DXGI_ALPHA_MODE_IGNORE;

				auto dxgiFactory = dxDev->GetDxgiFactory();
				Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgiSwapChain1;

				switch (this->initData.environment) {
				case InitData::Environment::Desktop: {
					hr = dxgiFactory->CreateSwapChainForHwnd(
						d3dDevice.Get(),
						this->initData.hWnd,
						&swapChainDesc,
						nullptr,
						nullptr,
						&dxgiSwapChain1
					);
					H::System::ThrowIfFailed(hr);

					hr = dxgiSwapChain1.As(&this->dxgiSwapChain);
					H::System::ThrowIfFailed(hr);
					break;
				}

				case InitData::Environment::UWP: {
					// When using XAML interop, the swap chain must be created for composition.
					hr = dxgiFactory->CreateSwapChainForComposition(
						d3dDevice.Get(),
						&swapChainDesc,
						nullptr,
						&dxgiSwapChain1
					);
					H::System::ThrowIfFailed(hr);

					hr = dxgiSwapChain1.As(&this->dxgiSwapChain);
					H::System::ThrowIfFailed(hr);

					if (LOG_ASSERT(this->initData.fnCreateSwapChainPannelDxgi, "this->fnCreateSwapChainPannelDxgi is empty")) {
						H::System::ThrowIfFailed(E_INVALIDARG);
					}
					this->initData.fnCreateSwapChainPannelDxgi(this->dxgiSwapChain.Get());
					break;
				}
				}

				// Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
				// ensures that the application will only render after each VSync, minimizing power consumption.
				Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
				hr = d3dDevice.As(&dxgiDevice);
				H::System::ThrowIfFailed(hr);

				hr = dxgiDevice->SetMaximumFrameLatency(1);
				H::System::ThrowIfFailed(hr);
			}

			// Set the proper orientation for the swap chain, and generate 2D and
			// 3D matrix transformations for rendering to the rotated swap chain.
			// Note the rotation angle for the 2D and 3D transforms are different.
			// This is due to the difference in coordinate spaces.  Additionally,
			// the 3D matrix is specified explicitly to avoid rounding errors.

			switch (displayRotation) {
			case DXGI_MODE_ROTATION_IDENTITY:
				this->m_orientationTransform2D = D2D1::Matrix3x2F::Identity();
				this->m_orientationTransform3D = ScreenRotation::Rotation0;
				break;

			case DXGI_MODE_ROTATION_ROTATE90:
				this->m_orientationTransform2D =
					D2D1::Matrix3x2F::Rotation(90.0f) *
					D2D1::Matrix3x2F::Translation(this->m_logicalSize.height, 0.0f);
				this->m_orientationTransform3D = ScreenRotation::Rotation270;
				break;

			case DXGI_MODE_ROTATION_ROTATE180:
				this->m_orientationTransform2D =
					D2D1::Matrix3x2F::Rotation(180.0f) *
					D2D1::Matrix3x2F::Translation(this->m_logicalSize.width, this->m_logicalSize.height);
				this->m_orientationTransform3D = ScreenRotation::Rotation180;
				break;

			case DXGI_MODE_ROTATION_ROTATE270:
				this->m_orientationTransform2D =
					D2D1::Matrix3x2F::Rotation(270.0f) *
					D2D1::Matrix3x2F::Translation(0.0f, this->m_logicalSize.width);
				this->m_orientationTransform3D = ScreenRotation::Rotation90;
				break;

			default:
				LOG_ERROR_D("'displayRotation' unspecified");
				throw std::exception{ "'displayRotation' unspecified" };
			}

			hr = this->dxgiSwapChain->SetRotation(displayRotation);
			H::System::ThrowIfFailed(hr);

			if (this->initData.environment == InitData::Environment::UWP) {
				// Setup inverse scale on the swap chain
				DXGI_MATRIX_3X2_F inverseScale = { 0 };
				inverseScale._11 = 1.0f / this->m_effectiveCompositionScaleX;
				inverseScale._22 = 1.0f / this->m_effectiveCompositionScaleY;
				Microsoft::WRL::ComPtr<IDXGISwapChain2> swapChain2;
				hr = this->dxgiSwapChain.As<IDXGISwapChain2>(&swapChain2);
				H::System::ThrowIfFailed(hr);

				hr = swapChain2->SetMatrixTransform(&inverseScale);
				H::System::ThrowIfFailed(hr);
			}

			// Handle color space settings for HDR
			this->UpdateColorSpace();
			
			// Create a render target view of the swap chain back buffer.
			hr = this->dxgiSwapChain->GetBuffer(0, IID_PPV_ARGS(&this->dxgiSwapChainBackBuffer));
			H::System::ThrowIfFailed(hr);

			CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(
				D3D11_RTV_DIMENSION_TEXTURE2D,
				backBufferFormat
			);
			hr = d3dDevice->CreateRenderTargetView(
				this->dxgiSwapChainBackBuffer.Get(),
				&renderTargetViewDesc, 
				&this->m_d3dRenderTargetView
			);
			H::System::ThrowIfFailed(hr);


			if (this->initData.depthBufferFormat != DXGI_FORMAT_UNKNOWN) {
				// Create a depth stencil view for use with 3D rendering if needed.
				CD3D11_TEXTURE2D_DESC1 depthStencilDesc(
					this->initData.depthBufferFormat, // DXGI_FORMAT_D24_UNORM_S8_UINT,
					lround(this->m_d3dRenderTargetSize.width),
					lround(this->m_d3dRenderTargetSize.height),
					1, // This depth stencil view has only one texture.
					1, // Use a single mipmap level.
					D3D11_BIND_DEPTH_STENCIL
				);

				Microsoft::WRL::ComPtr<ID3D11Texture2D1> depthStencil;
				hr = d3dDevice->CreateTexture2D1(
					&depthStencilDesc,
					nullptr,
					&depthStencil
				);
				H::System::ThrowIfFailed(hr);

				CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
				hr = d3dDevice->CreateDepthStencilView(
					depthStencil.Get(),
					&depthStencilViewDesc,
					&this->m_d3dDepthStencilView
				);
				H::System::ThrowIfFailed(hr);
			}

			// Set the 3D rendering viewport to target the entire window.
			this->m_screenViewport = CD3D11_VIEWPORT(
				0.0f,
				0.0f,
				this->m_d3dRenderTargetSize.width,
				this->m_d3dRenderTargetSize.height
			);
			d3dCtx->RSSetViewports(1, &this->m_screenViewport);

			d2dCtx->SetDpi(this->m_effectiveDpi, this->m_effectiveDpi);

			// Grayscale text anti-aliasing is recommended for all Microsoft Store apps.
			d2dCtx->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
			

			if (auto dxSettings = this->initData.dxSettingsWeak.lock()) {
				if (dxSettings->IsMSAAEnabled()) {
					//	D3D11_TEXTURE2D_DESC desc = {};
					//	this->dxgiSwapChainBackBuffer->GetDesc(&desc);

					//	auto availableMsaaLevel = H::Dx::MsaaHelper::GetMaxSupportedMSAA(d3dDevice.Get(), desc.Format, H::Dx::MsaaHelper::GetMaxMSAA());
					//	if (availableMsaaLevel) {
					//		desc.SampleDesc = *availableMsaaLevel;

					//		hr = d3dDevice->CreateTexture2D(&desc, nullptr, &this->msaaTexture);
					//		H::System::ThrowIfFailed(hr);

					//		hr = d3dDevice->CreateRenderTargetView1(this->msaaTexture.Get(), nullptr, &this->msaaRenderTargetView);
					//		H::System::ThrowIfFailed(hr);
					//	}
				}
			}


			// Create a Direct2D target bitmap associated with the
			// swap chain back buffer and set it as the current target.
			DXGI_FORMAT btimapFormat;
			Microsoft::WRL::ComPtr<IDXGISurface2> dxgiSurface;

			// TODO: dxRenderProxy texture make sense if you need to use 'd2dTargetBitmap'.
			//       Add flag "SupportD2D" and make this logic optional. For complex render,
			//	     disabling d2d support will help impove performance.
			switch (backBufferFormat) {
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_R16G16B16A16_FLOAT: {
				// These swapChain formats are compatible with CreateBitmapFromDxgiSurface().
				btimapFormat = backBufferFormat;
				hr = this->dxgiSwapChainBackBuffer.As(&dxgiSurface);
				H::System::ThrowIfFailed(hr);
				break;
			}

			default: {
				btimapFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

				if (!this->dxRenderObjProxy) {
					this->dxRenderObjProxy = std::make_unique<details::DxRenderObjProxy>(this, btimapFormat);
				}
				if (!this->renderPipeline) {
					this->renderPipeline = std::make_unique<RenderPipeline>(&this->dxDeviceSafeObj);
				}

				this->dxRenderObjProxy->CreateWindowSizeDependentResources();

				hr = this->dxRenderObjProxy->GetObjData()->texture.As(&dxgiSurface);
				H::System::ThrowIfFailed(hr);
				break;
			}
			}

			D2D1_BITMAP_PROPERTIES1 bitmapProperties =
				D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
					D2D1::PixelFormat(btimapFormat, D2D1_ALPHA_MODE_PREMULTIPLIED),
					this->m_dpi,
					this->m_dpi
				);

			hr = d2dCtx->CreateBitmapFromDxgiSurface(
				dxgiSurface.Get(),
				&bitmapProperties, // if nullptr - use properties from dxgiSurface
				&this->m_d2dTargetBitmap
			);
			H::System::ThrowIfFailed(hr);

			d2dCtx->SetTarget(this->m_d2dTargetBitmap.Get());
		}

		// Determine the dimensions of the render target and whether it will be scaled down.
		void SwapChainPanel::UpdateRenderTargetSize() {
			this->m_effectiveDpi = this->m_dpi;
			this->m_effectiveCompositionScaleX = this->m_compositionScaleX;
			this->m_effectiveCompositionScaleY = this->m_compositionScaleY;

			// To improve battery life on high resolution devices, render to a smaller render target
			// and allow the GPU to scale the output when it is presented.
			if (!DisplayMetrics::SupportHighResolutions && this->m_dpi > DisplayMetrics::DpiThreshold)
			{
				float width = H::Dx::ConvertDipsToPixels(this->m_logicalSize.width, this->m_dpi);
				float height = H::Dx::ConvertDipsToPixels(this->m_logicalSize.height, this->m_dpi);

				// When the device is in portrait orientation, height > width. Compare the
				// larger dimension against the width threshold and the smaller dimension
				// against the height threshold.
				//if (std::max(width, height) > DisplayMetrics::WidthThreshold && std::min(width, height) > DisplayMetrics::HeightThreshold)
				//{
					// To scale the app we change the effective DPI. Logical size does not change.
				this->m_effectiveDpi /= this->m_resolutionScale;
				this->m_effectiveCompositionScaleX /= this->m_resolutionScale;
				this->m_effectiveCompositionScaleY /= this->m_resolutionScale;
				//}
			}

			// Calculate the necessary render target size in pixels.
			this->m_outputSize.width = H::Dx::ConvertDipsToPixels(this->m_logicalSize.width, this->m_effectiveDpi);
			this->m_outputSize.height = H::Dx::ConvertDipsToPixels(this->m_logicalSize.height, this->m_effectiveDpi);

			// Prevent zero size DirectX content from being created.
			this->m_outputSize.width = std::max(this->m_outputSize.width, 1.0f);
			this->m_outputSize.height = std::max(this->m_outputSize.height, 1.0f);
		}

		// This method is called in the event handler for the SizeChanged event.
		void STDMETHODCALLTYPE SwapChainPanel::SetLogicalSize(H::Size_f logicalSize) {
			if (this->m_logicalSize != logicalSize) {
				this->m_logicalSize = logicalSize;
				this->CreateWindowSizeDependentResources();
			}
		}

		// This method is called in the event handler for the DpiChanged event.
		void STDMETHODCALLTYPE SwapChainPanel::SetDpi(float dpi) {
			auto dxDev = this->dxDeviceSafeObj.Lock();
			auto dxCtx = dxDev->LockContext();
			auto d2dCtx = dxCtx->D2D();

			if (dpi != this->m_dpi) {
				this->m_dpi = dpi;
				d2dCtx->SetDpi(this->m_dpi, this->m_dpi);
				this->CreateWindowSizeDependentResources();
			}
		}

		void STDMETHODCALLTYPE SwapChainPanel::SetNativeOrientation(DisplayOrientations nativeOrientation) {
			this->m_nativeOrientation = nativeOrientation;
		}

		// This method is called in the event handler for the OrientationChanged event.
		void STDMETHODCALLTYPE SwapChainPanel::SetCurrentOrientation(DisplayOrientations currentOrientation) {
			if (this->m_currentOrientation != currentOrientation) {
				this->m_currentOrientation = currentOrientation;
				this->CreateWindowSizeDependentResources();
			}
		}

		// This method is called in the event handler for the CompositionScaleChanged event.
		void STDMETHODCALLTYPE SwapChainPanel::SetCompositionScale(float compositionScaleX, float compositionScaleY) {
			if (this->m_compositionScaleX != compositionScaleX ||
				this->m_compositionScaleY != compositionScaleY)
			{
				this->m_compositionScaleX = compositionScaleX;
				this->m_compositionScaleY = compositionScaleY;
				this->CreateWindowSizeDependentResources();
			}
		}

		void STDMETHODCALLTYPE SwapChainPanel::SetRenderResolutionScale(float resolutionScale) {
			if (this->m_resolutionScale != resolutionScale) {
				this->m_resolutionScale = resolutionScale;
				this->CreateWindowSizeDependentResources();
			}
		}

		// This method is called in the event handler for the DisplayContentsInvalidated event.
		void STDMETHODCALLTYPE SwapChainPanel::ValidateDevice() {
			HRESULT hr = S_OK;
			auto dxDev = this->dxDeviceSafeObj.Lock();
			auto d3dDevice = dxDev->GetD3DDevice();

			// The D3D Device is no longer valid if the default adapter changed since the device
			// was created or if the device has been removed.

			// First, get the information for the default adapter from when the device was created.

			Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
			hr = d3dDevice.As(&dxgiDevice);
			H::System::ThrowIfFailed(hr);

			Microsoft::WRL::ComPtr<IDXGIAdapter> deviceAdapter;
			hr = dxgiDevice->GetAdapter(&deviceAdapter);
			H::System::ThrowIfFailed(hr);

			Microsoft::WRL::ComPtr<IDXGIFactory2> deviceFactory;
			hr = deviceAdapter->GetParent(IID_PPV_ARGS(&deviceFactory));
			H::System::ThrowIfFailed(hr);

			Microsoft::WRL::ComPtr<IDXGIAdapter1> previousDefaultAdapter;
			hr = deviceFactory->EnumAdapters1(0, &previousDefaultAdapter);
			H::System::ThrowIfFailed(hr);

			DXGI_ADAPTER_DESC1 previousDesc;
			hr = previousDefaultAdapter->GetDesc1(&previousDesc);
			H::System::ThrowIfFailed(hr);

			// Next, get the information for the current default adapter.

			Microsoft::WRL::ComPtr<IDXGIFactory4> currentFactory;
			hr = CreateDXGIFactory1(IID_PPV_ARGS(&currentFactory));
			H::System::ThrowIfFailed(hr);

			Microsoft::WRL::ComPtr<IDXGIAdapter1> currentDefaultAdapter;
			hr = currentFactory->EnumAdapters1(0, &currentDefaultAdapter);
			H::System::ThrowIfFailed(hr);

			DXGI_ADAPTER_DESC1 currentDesc;
			hr = currentDefaultAdapter->GetDesc1(&currentDesc);
			H::System::ThrowIfFailed(hr);

			// If the adapter LUIDs don't match, or if the device reports that it has been removed,
			// a new D3D device must be created.

			if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
				previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
				FAILED(d3dDevice->GetDeviceRemovedReason()))
			{
				// Release references to resources related to the old device.
				dxgiDevice = nullptr;
				deviceAdapter = nullptr;
				deviceFactory = nullptr;
				previousDefaultAdapter = nullptr;

				// Create a new device and swap chain.
				this->HandleDeviceLost();
			}
		}

		// Recreate all device resources and set them back to the current state.
		void STDMETHODCALLTYPE SwapChainPanel::HandleDeviceLost() {
			assert(false);
			// TODO: implement
			//auto dxDev = this->dxDeviceSafeObj.Lock();

			//this->dxgiSwapChain = nullptr;

			//if (this->m_deviceNotify != nullptr) {
			//	this->m_deviceNotify->OnDeviceLost();
			//}

			//CreateDeviceResources();
			//this->m_d2dContext->SetDpi(this->m_dpi, this->m_dpi);
			//CreateWindowSizeDependentResources();

			//if (this->m_deviceNotify != nullptr) {
			//	this->m_deviceNotify->OnDeviceRestored();
			//}
		}

		// Call this method when the app suspends. It provides a hint to the driver that the app 
		// is entering an idle state and that temporary buffers can be reclaimed for use by other apps.
		void STDMETHODCALLTYPE SwapChainPanel::Trim() {
			auto dxDev = this->dxDeviceSafeObj.Lock();
			auto d3dDevice = dxDev->GetD3DDevice();

			Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
			d3dDevice.As(&dxgiDevice);

			dxgiDevice->Trim();
		}

		// Present the contents of the swap chain to the screen.
		void STDMETHODCALLTYPE SwapChainPanel::Present() {
			HRESULT hr = S_OK;

			auto dxDev = this->dxDeviceSafeObj.Lock();
			auto dxCtx = dxDev->LockContext();
			auto d3dCtx = dxCtx->D3D();

			bool isMSAA = false;
			bool isVSync = true;
			if (auto dxSettings = this->initData.dxSettingsWeak.lock()) {
				isMSAA = dxSettings->IsMSAAEnabled();
				isVSync = dxSettings->IsVSyncEnabled();
			}

			if (isMSAA) {
				//if (this->msaaTexture) {
				//	d3dCtx->ResolveSubresource(this->dxgiSwapChainBackBuffer.Get(), 0, this->msaaTexture.Get(), 0, this->initData.backBufferFormat);
				//}
			}

			if (this->dxRenderObjProxy) {
				this->DrawProxyTextureToSwapChainRTV();
			}

			// The first argument instructs DXGI to block until VSync, putting the application
			// to sleep until the next VSync. This ensures we don't waste any cycles rendering
			// frames that will never be displayed to the screen.
			DXGI_PRESENT_PARAMETERS parameters = { 0 };
			hr = this->dxgiSwapChain->Present1(isVSync, 0, &parameters);

			this->swapChainPanelNotifications.onPresent.Invoke();			

			// Discard the contents of the render target.
			// This is a valid operation only when the existing contents will be entirely
			// overwritten. If dirty or scroll rects are used, this call should be modified.
			if (this->dxRenderObjProxy) {
				d3dCtx->DiscardView1(this->dxRenderObjProxy->GetObjData()->textureRTV.Get(), nullptr, 0);
			}
			d3dCtx->DiscardView1(this->m_d3dRenderTargetView.Get(), nullptr, 0);

			if (this->m_d3dDepthStencilView) {
				// Discard the contents of the depth stencil.
				d3dCtx->DiscardView1(this->m_d3dDepthStencilView.Get(), nullptr, 0);
			}

			if (isMSAA) {
				//if (this->msaaRenderTargetView) {
				//	d3dCtx->DiscardView1(this->msaaRenderTargetView.Get(), nullptr, 0);
				//}
			}

			// If the device was removed either by a disconnection or a driver upgrade, we 
			// must recreate all device resources.
			if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
				this->HandleDeviceLost();
			}
			else {
				H::System::ThrowIfFailed(hr);
			}
		}

		// Register our DeviceNotify to be informed on device lost and creation.
		void STDMETHODCALLTYPE SwapChainPanel::RegisterDeviceNotify(IDeviceNotify* deviceNotify) {
			this->deviceNotify = deviceNotify;
		}

		//void STDMETHODCALLTYPE SwapChainPanel::RegisterRenderNotification(IRenderNotification* renderNotification) {
		//	this->renderNotification = renderNotification;
		//}

		SwapChainPanelNotifications* STDMETHODCALLTYPE SwapChainPanel::GetNotifications() {
			return &this->swapChainPanelNotifications;
		}


		H::Size_f STDMETHODCALLTYPE SwapChainPanel::GetOutputSize() const {
			return this->m_outputSize;
		}

		H::Size_f STDMETHODCALLTYPE SwapChainPanel::GetLogicalSize() const {
			return this->m_logicalSize;
		}

		H::Size_f STDMETHODCALLTYPE SwapChainPanel::GetRenderTargetSize() const {
			return this->m_d3dRenderTargetSize;
		}

		DisplayOrientations STDMETHODCALLTYPE SwapChainPanel::GetNativeOrientation() const {
			return this->m_nativeOrientation;
		}

		DisplayOrientations STDMETHODCALLTYPE SwapChainPanel::GetCurrentOrientation() const {
			return this->m_currentOrientation;
		}

		float STDMETHODCALLTYPE SwapChainPanel::GetDpi() const {
			return this->m_effectiveDpi;
		}

		DirectX::XMFLOAT2 STDMETHODCALLTYPE SwapChainPanel::GetCompositionScale() const {
			return { this->m_compositionScaleX, this->m_compositionScaleY };
		}


		std::ex::weak_ptr<HELPERS_NS::Dx::DxSettings> STDMETHODCALLTYPE SwapChainPanel::GetDxSettings() const {
			return this->initData.dxSettingsWeak;
		}

		Microsoft::WRL::ComPtr<IDXGISwapChain3> STDMETHODCALLTYPE SwapChainPanel::GetSwapChain() const {
			return this->dxgiSwapChain;
		}

		// TODO: add logic to return msaaTexture render view.
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> STDMETHODCALLTYPE SwapChainPanel::GetRenderTargetView() const {
			if (this->dxRenderObjProxy) {
				return this->dxRenderObjProxy->GetObjData()->textureRTV;
			}
			return this->m_d3dRenderTargetView;
		}

		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> STDMETHODCALLTYPE SwapChainPanel::GetDepthStencilView() const {
			return this->m_d3dDepthStencilView;
		}

		D3D11_VIEWPORT STDMETHODCALLTYPE SwapChainPanel::GetScreenViewport() const {
			return this->m_screenViewport;
		}

		DirectX::XMFLOAT4X4 STDMETHODCALLTYPE SwapChainPanel::GetOrientationTransform3D() const {
			return this->m_orientationTransform3D;
		}

		Microsoft::WRL::ComPtr<ID2D1Bitmap1>STDMETHODCALLTYPE  SwapChainPanel::GetD2DTargetBitmap() const {
			return this->m_d2dTargetBitmap;
		}

		D2D1::Matrix3x2F STDMETHODCALLTYPE SwapChainPanel::GetOrientationTransform2D() const {
			return this->m_orientationTransform2D;
		}

		DXGI_COLOR_SPACE_TYPE STDMETHODCALLTYPE SwapChainPanel::GetColorSpace() const {
			return this->colorSpace;
		}

		bool STDMETHODCALLTYPE SwapChainPanel::IsDisplayHDR10() const {
			return this->isDisplayHDR10;
		}



		// This method determines the rotation between the display device's native orientation and the
		// current display orientation.
		DXGI_MODE_ROTATION SwapChainPanel::ComputeDisplayRotation() {
			DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

			// Note: NativeOrientation can only be Landscape or Portrait even though
			// the DisplayOrientations enum has other values.
			switch (this->m_nativeOrientation) {
			case DisplayOrientations::Landscape:
				switch (this->m_currentOrientation)
				{
				case DisplayOrientations::Landscape:
					rotation = DXGI_MODE_ROTATION_IDENTITY;
					break;

				case DisplayOrientations::Portrait:
					rotation = DXGI_MODE_ROTATION_ROTATE270;
					break;

				case DisplayOrientations::LandscapeFlipped:
					rotation = DXGI_MODE_ROTATION_ROTATE180;
					break;

				case DisplayOrientations::PortraitFlipped:
					rotation = DXGI_MODE_ROTATION_ROTATE90;
					break;
				}
				break;

			case DisplayOrientations::Portrait:
				switch (this->m_currentOrientation)
				{
				case DisplayOrientations::Landscape:
					rotation = DXGI_MODE_ROTATION_ROTATE90;
					break;

				case DisplayOrientations::Portrait:
					rotation = DXGI_MODE_ROTATION_IDENTITY;
					break;

				case DisplayOrientations::LandscapeFlipped:
					rotation = DXGI_MODE_ROTATION_ROTATE270;
					break;

				case DisplayOrientations::PortraitFlipped:
					rotation = DXGI_MODE_ROTATION_ROTATE180;
					break;
				}
				break;
			}
			return rotation;
		}

		// TODO: call this method when core window moved.
		// Sets the color space for the swap chain in order to handle HDR output.
		void SwapChainPanel::UpdateColorSpace() {
			HRESULT hr = S_OK;
			auto dxDev = this->dxDeviceSafeObj.Lock();
			auto d3dDevice = dxDev->GetD3DDevice();
			auto dxgiFactory = dxDev->GetDxgiFactory();

			if (!dxgiFactory->IsCurrent()) {
				// Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
				dxDev->CreateDxgiFactory();
			}

			DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
			this->isDisplayHDR10 = false;

#if defined(NTDDI_WIN10_RS2)
			if (this->dxgiSwapChain) {
				// To detect HDR support, we will need to check the color space in the primary
				// DXGI output associated with the app at this point in time
				// (using window/display intersection).

				// Get the retangle bounds of the app window.
				RECT windowBounds;

#if COMPILE_FOR_DESKTOP
				HWND dxgiSwapChainHwnd;
				hr = this->dxgiSwapChain->GetHwnd(&dxgiSwapChainHwnd);
				H::System::ThrowIfFailed(hr);

				if (!GetWindowRect(dxgiSwapChainHwnd, &windowBounds)) {
					LogLastError;
					throw std::system_error(std::error_code(static_cast<int>(GetLastError()), std::system_category()), "GetWindowRect");
				}

#elif COMPILE_FOR_CX_or_WINRT
				if (LOG_ASSERT(this->initData.fnGetWindowBounds, "this->fnGetWindowBounds is empty")) {
					H::System::ThrowIfFailed(E_INVALIDARG);
				}

				auto coreWindowBounds = this->initData.fnGetWindowBounds();
				windowBounds = RECT{
					static_cast<long>(coreWindowBounds.left),
					static_cast<long>(coreWindowBounds.top),
					static_cast<long>(coreWindowBounds.right),
					static_cast<long>(coreWindowBounds.bottom),
				};
#endif

				const long ax1 = windowBounds.left;
				const long ay1 = windowBounds.top;
				const long ax2 = windowBounds.right;
				const long ay2 = windowBounds.bottom;

				Microsoft::WRL::ComPtr<IDXGIOutput> bestOutput;
				long bestIntersectArea = -1;

				Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
				for (UINT adapterIndex = 0;
					SUCCEEDED(dxgiFactory->EnumAdapters(adapterIndex, adapter.ReleaseAndGetAddressOf()));
					++adapterIndex)
				{
					Microsoft::WRL::ComPtr<IDXGIOutput> output;
					for (UINT outputIndex = 0;
						SUCCEEDED(adapter->EnumOutputs(outputIndex, output.ReleaseAndGetAddressOf()));
						++outputIndex)
					{
						// Get the rectangle bounds of current output.
						DXGI_OUTPUT_DESC desc;
						hr = output->GetDesc(&desc);
						H::System::ThrowIfFailed(hr);

						// Compute the intersection
						const auto& r = desc.DesktopCoordinates;
						const long intersectArea = ::Tools::ComputeIntersectionArea(ax1, ay1, ax2, ay2, r.left, r.top, r.right, r.bottom);
						if (intersectArea > bestIntersectArea) {
							bestOutput.Swap(output);
							bestIntersectArea = intersectArea;
						}
					}
				}

				if (bestOutput) {
					Microsoft::WRL::ComPtr<IDXGIOutput6> output6;
					if (SUCCEEDED(bestOutput.As(&output6))) {
						DXGI_OUTPUT_DESC1 desc;
						hr = output6->GetDesc1(&desc);
						H::System::ThrowIfFailed(hr);

						if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020) {
							// Display output is HDR10.
							this->isDisplayHDR10 = true;
						}
					}
				}
			}
#endif

			if (this->initData.optionFlags.Has(InitData::Options::EnableHDR) && this->isDisplayHDR10) {
				switch (this->initData.backBufferFormat) {
				case DXGI_FORMAT_R10G10B10A2_UNORM:
					// The application creates the HDR10 signal.
					colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
					break;

				case DXGI_FORMAT_R16G16B16A16_FLOAT:
					// The system creates the HDR10 signal; application uses linear values.
					colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
					break;

				default:
					break;
				}
			}

			this->colorSpace = colorSpace;

			Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain3;
			if (this->dxgiSwapChain && SUCCEEDED(this->dxgiSwapChain.As(&swapChain3)))
			{
				UINT colorSpaceSupport = 0;
				if (SUCCEEDED(swapChain3->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport))
					&& (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
				{
					hr = swapChain3->SetColorSpace1(colorSpace);
					H::System::ThrowIfFailed(hr);
				}
			}
		}

		void SwapChainPanel::DrawProxyTextureToSwapChainRTV() {
			HRESULT hr = S_OK;

			auto dxDev = this->dxDeviceSafeObj.Lock();
			auto dxCtx = dxDev->LockContext();
			auto d3dCtx = dxCtx->D3D();

			// Render ObjProxy texture (for example 8:8:8:8) to swapChain RTV (for example 10:10:10:2).
			auto renderTargetView = this->m_d3dRenderTargetView;
			d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Aqua);

			ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
			d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

			auto viewport = this->GetScreenViewport();
			d3dCtx->RSSetViewports(1, &viewport);

			auto& dxRenderObj = this->dxRenderObjProxy;
			dxRenderObj->UpdateBuffers();

			this->renderPipeline->SetTexture(dxRenderObj->GetObjData()->textureSRV);
			this->renderPipeline->SetVertexShader(
				dxRenderObj->GetObjData()->vertexShader,
				dxRenderObj->GetObjData()->vsConstantBuffer
			);
			this->renderPipeline->SetPixelShader(
				dxRenderObj->GetObjData()->pixelShader,
				dxRenderObj->GetObjData()->psConstantBuffer
			);

			this->renderPipeline->Draw();

			ID3D11ShaderResourceView* nullrtv[] = { nullptr };
			d3dCtx->PSSetShaderResources(0, _countof(nullrtv), nullrtv);
		}
	}
}