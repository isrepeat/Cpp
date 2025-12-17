#pragma once
#include "Helpers/common.h"
#include "Helpers/Std/Extensions/memoryEx.h"
#include "Helpers/Dx/DxSettings.h"
#include "Helpers/Dx/DxDevice.h"
#include "Helpers/Event/Signal.h"

#include <Unknwn.h>
#include <memory>

namespace HELPERS_NS {
	namespace Dx {
		// Provides an interface for an application that owns DeviceResources to be notified of the device being lost or created.
		interface IDeviceNotify {
			virtual void OnDeviceLost() = 0;
			virtual void OnDeviceRestored() = 0;
		};

		struct SwapChainPanelNotifications {
			// CHECK: if Signal class have std::mutex then may be unexpected errors, see ComMutex notes for it
			HELPERS_NS::Event::Signal<void()> onPresent;
		};

		enum DisplayOrientations : unsigned int {
			None = 0,
			Landscape = 0x1,
			Portrait = 0x2,
			LandscapeFlipped = 0x4,
			PortraitFlipped = 0x8,
		};


		// [Guid("B34F96A3-8A71-42D3-BE89-C09777D1BB99")]
		MIDL_INTERFACE("B34F96A3-8A71-42D3-BE89-C09777D1BB99")
			ISwapChainPanel : public IUnknown{
			public:
				virtual HELPERS_NS::Dx::DxDeviceSafeObj* STDMETHODCALLTYPE GetDxDevice() = 0;

				virtual void STDMETHODCALLTYPE InitSwapChainPanelInfo(
					HELPERS_NS::Size_f logicalSize,
					DisplayOrientations nativeOrientation,
					DisplayOrientations currentOrientation,
					float compositionScaleX,
					float compositionScaleY,
					float dpi
				) = 0;


				virtual void STDMETHODCALLTYPE SetLogicalSize(HELPERS_NS::Size_f logicalSize) = 0;
				virtual void STDMETHODCALLTYPE SetNativeOrientation(DisplayOrientations nativeOrientation) = 0;
				virtual void STDMETHODCALLTYPE SetCurrentOrientation(DisplayOrientations currentOrientation) = 0;
				virtual void STDMETHODCALLTYPE SetDpi(float dpi) = 0;
				virtual void STDMETHODCALLTYPE SetCompositionScale(float compositionScaleX, float compositionScaleY) = 0;
				virtual void STDMETHODCALLTYPE SetRenderResolutionScale(float resolutionScale) = 0;
				virtual void STDMETHODCALLTYPE ValidateDevice() = 0;
				virtual void STDMETHODCALLTYPE HandleDeviceLost() = 0;
				virtual void STDMETHODCALLTYPE Trim() = 0;
				virtual void STDMETHODCALLTYPE Present() = 0;
				virtual void STDMETHODCALLTYPE RegisterDeviceNotify(IDeviceNotify* deviceNotify) = 0;

				// TODO: mb rewrite SwapChainPanelNotifications to return ComPtr on it?
				virtual SwapChainPanelNotifications* STDMETHODCALLTYPE GetNotifications() = 0;

				// The size of the render target, in pixels.
				virtual HELPERS_NS::Size_f STDMETHODCALLTYPE GetOutputSize() const = 0;

				// The size of the render target, in dips.
				virtual HELPERS_NS::Size_f STDMETHODCALLTYPE GetLogicalSize() const = 0;
				virtual HELPERS_NS::Size_f STDMETHODCALLTYPE GetRenderTargetSize() const = 0;
				virtual DisplayOrientations STDMETHODCALLTYPE GetNativeOrientation() const = 0;
				virtual DisplayOrientations STDMETHODCALLTYPE GetCurrentOrientation() const = 0;
				virtual float STDMETHODCALLTYPE GetDpi() const = 0;
				virtual DirectX::XMFLOAT2 STDMETHODCALLTYPE GetCompositionScale() const = 0;

				virtual STD_EXT_NS::weak_ptr<HELPERS_NS::Dx::DxSettings> STDMETHODCALLTYPE GetDxSettings() const = 0;

				// D3D Accessors.
				virtual Microsoft::WRL::ComPtr<IDXGISwapChain3> STDMETHODCALLTYPE GetSwapChain() const = 0;
				virtual Microsoft::WRL::ComPtr<ID3D11RenderTargetView> STDMETHODCALLTYPE GetRenderTargetView() const = 0;
				virtual Microsoft::WRL::ComPtr<ID3D11DepthStencilView> STDMETHODCALLTYPE GetDepthStencilView() const = 0;
				virtual D3D11_VIEWPORT STDMETHODCALLTYPE GetScreenViewport() const = 0;
				virtual DirectX::XMFLOAT4X4	STDMETHODCALLTYPE GetOrientationTransform3D() const = 0;

				// D2D Accessors.
				virtual Microsoft::WRL::ComPtr<ID2D1Bitmap1> STDMETHODCALLTYPE GetD2DTargetBitmap() const = 0;
				virtual D2D1::Matrix3x2F STDMETHODCALLTYPE GetOrientationTransform2D() const = 0;

				virtual DXGI_COLOR_SPACE_TYPE STDMETHODCALLTYPE GetColorSpace() const = 0;
				virtual bool STDMETHODCALLTYPE IsDisplayHDR10() const = 0;
		};
	}
}