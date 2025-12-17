#pragma once
#include "Helpers/common.h"
#include "Helpers/Callback.hpp"
#include "Helpers/Flags.h"

#include "ISwapChainPanel.h"
#include "DxRenderObjProxy.h"
#include "RenderPipeline.h"

namespace HELPERS_NS {
	namespace Dx {
#define _Enum_SwapChainPanelInitData_Environment \
	Desktop, \
	UWP,

#define _Enum_SwapChainPanelInitData_Options \
	None = 0x01, \
	EnableHDR = 0x02,

		// Controls all the DirectX device resources.
		class SwapChainPanel : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<
			Microsoft::WRL::RuntimeClassType::ClassicCom>,
			ISwapChainPanel>
		{
		public:
			struct InitData {
				enum class Environment {
					_Enum_SwapChainPanelInitData_Environment
				};
				enum Options {
					_Enum_SwapChainPanelInitData_Options
				};

				Environment environment = Environment::Desktop;
				HELPERS_NS::Flags<Options> optionFlags = Options::None;

				std::function<std::unique_ptr<HELPERS_NS::Dx::details::DxDevice>()> dxDeviceFactory = [] {
					return std::make_unique<HELPERS_NS::Dx::details::DxDevice>();
				};
				std::function<std::unique_ptr<HELPERS_NS::IMutex>()> dxDeviceSafeObjMutexFactory = [] {
					return std::make_unique<HELPERS_NS::Mutex<std::recursive_mutex>>();
				};

				DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
				DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_UNKNOWN;
				DXGI_SWAP_EFFECT dxgiSwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

				HWND hWnd = nullptr;
				HELPERS_NS::Callback<void, IDXGISwapChain3*> fnCreateSwapChainPannelDxgi = {};
				HELPERS_NS::Callback<HELPERS_NS::Rect> fnGetWindowBounds = {};

				std::weak_ptr<HELPERS_NS::Dx::DxSettings> dxSettingsWeak;
			};

			SwapChainPanel(const InitData& initData);
			~SwapChainPanel();

			HELPERS_NS::Dx::DxDeviceSafeObj* STDMETHODCALLTYPE GetDxDevice() override;

			void STDMETHODCALLTYPE InitSwapChainPanelInfo(
				HELPERS_NS::Size_f logicalSize,
				DisplayOrientations nativeOrientation,
				DisplayOrientations currentOrientation,
				float compositionScaleX,
				float compositionScaleY,
				float dpi
			) override;

			void STDMETHODCALLTYPE SetLogicalSize(HELPERS_NS::Size_f logicalSize) override;
			void STDMETHODCALLTYPE SetNativeOrientation(DisplayOrientations nativeOrientation) override;
			void STDMETHODCALLTYPE SetCurrentOrientation(DisplayOrientations currentOrientation) override;
			void STDMETHODCALLTYPE SetDpi(float dpi) override;
			void STDMETHODCALLTYPE SetCompositionScale(float compositionScaleX, float compositionScaleY) override;
			void STDMETHODCALLTYPE SetRenderResolutionScale(float resolutionScale) override;
			void STDMETHODCALLTYPE ValidateDevice() override;
			void STDMETHODCALLTYPE HandleDeviceLost() override;
			void STDMETHODCALLTYPE Trim() override;
			void STDMETHODCALLTYPE Present() override;
			void STDMETHODCALLTYPE RegisterDeviceNotify(IDeviceNotify* deviceNotify) override;
			
			SwapChainPanelNotifications* STDMETHODCALLTYPE GetNotifications() override;

			// The size of the render target, in pixels.
			HELPERS_NS::Size_f STDMETHODCALLTYPE GetOutputSize() const override;

			// The size of the render target, in dips.
			HELPERS_NS::Size_f STDMETHODCALLTYPE GetLogicalSize() const override;
			HELPERS_NS::Size_f STDMETHODCALLTYPE GetRenderTargetSize() const override;
			DisplayOrientations STDMETHODCALLTYPE GetNativeOrientation() const override;
			DisplayOrientations STDMETHODCALLTYPE GetCurrentOrientation() const override;
			float STDMETHODCALLTYPE GetDpi() const override;
			DirectX::XMFLOAT2 STDMETHODCALLTYPE GetCompositionScale() const override;
			
			STD_EXT_NS::weak_ptr<HELPERS_NS::Dx::DxSettings> STDMETHODCALLTYPE GetDxSettings() const override;

			// D3D Accessors.
			Microsoft::WRL::ComPtr<IDXGISwapChain3> STDMETHODCALLTYPE GetSwapChain() const override;
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> STDMETHODCALLTYPE GetRenderTargetView() const override;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilView> STDMETHODCALLTYPE GetDepthStencilView() const override;
			D3D11_VIEWPORT STDMETHODCALLTYPE GetScreenViewport() const override;
			DirectX::XMFLOAT4X4	STDMETHODCALLTYPE GetOrientationTransform3D() const override;

			// D2D Accessors.
			Microsoft::WRL::ComPtr<ID2D1Bitmap1> STDMETHODCALLTYPE GetD2DTargetBitmap() const override;
			D2D1::Matrix3x2F STDMETHODCALLTYPE GetOrientationTransform2D() const override;

			DXGI_COLOR_SPACE_TYPE STDMETHODCALLTYPE GetColorSpace() const override;
			bool STDMETHODCALLTYPE IsDisplayHDR10() const override;

		private:
			void CreateWindowSizeDependentResources();
			void UpdateRenderTargetSize();
			DXGI_MODE_ROTATION ComputeDisplayRotation();
			void UpdateColorSpace();
			void DrawProxyTextureToSwapChainRTV();


		private:
			const InitData initData;

			// Direct3D objects.
			HELPERS_NS::Dx::DxDeviceSafeObj dxDeviceSafeObj;

			// Direct3D rendering objects. Required for 3D.
			Microsoft::WRL::ComPtr<IDXGISwapChain3>	dxgiSwapChain;
			Microsoft::WRL::ComPtr<ID3D11Texture2D> dxgiSwapChainBackBuffer;
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_d3dRenderTargetView;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_d3dDepthStencilView;

			//Microsoft::WRL::ComPtr<ID3D11Texture2D> msaaTexture;
			//Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> msaaRenderTargetView;

			// Direct2D drawing components.
			Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_d2dTargetBitmap;

			// Direct3D render piplines objects.
			std::unique_ptr<details::DxRenderObjProxy> dxRenderObjProxy;
			std::unique_ptr<RenderPipeline> renderPipeline;


			// Cached device properties.
			D3D11_VIEWPORT m_screenViewport;
			HELPERS_NS::Size_f m_d3dRenderTargetSize;
			HELPERS_NS::Size_f m_outputSize;
			HELPERS_NS::Size_f m_logicalSize;
			DisplayOrientations m_nativeOrientation;
			DisplayOrientations m_currentOrientation;
			float m_dpi;
			float m_compositionScaleX;
			float m_compositionScaleY;

			// Variables that take into account whether the app supports high resolution screens or not.
			float m_effectiveDpi;
			float m_effectiveCompositionScaleX;
			float m_effectiveCompositionScaleY;
			float m_resolutionScale;

			// Transforms used for display orientation.
			D2D1::Matrix3x2F m_orientationTransform2D;
			DirectX::XMFLOAT4X4	m_orientationTransform3D;

			// HDR Support
			DXGI_COLOR_SPACE_TYPE colorSpace;
			bool isDisplayHDR10;

			// The IDeviceNotify can be held directly as it owns the DeviceResources.
			IDeviceNotify* deviceNotify;
			SwapChainPanelNotifications swapChainPanelNotifications;
		};
	}

	//
	// Dx Tools
	//
#if COMPILE_FOR_WINRT
	namespace Dx {
		namespace WinRt {
			namespace Tools {
				inline Microsoft::WRL::ComPtr<HELPERS_NS::Dx::ISwapChainPanel> QuerySwapChainPanelNative(Platform::Object^ objSwapChainPanelWinRt) {
					HRESULT hr = S_OK;
					auto unk = reinterpret_cast<IUnknown*>(objSwapChainPanelWinRt);

					Microsoft::WRL::ComPtr<HELPERS_NS::Dx::ISwapChainPanel> swapChainPanelNative;
					hr = unk->QueryInterface(swapChainPanelNative.ReleaseAndGetAddressOf());
					HELPERS_NS::System::ThrowIfFailed(hr);

					return swapChainPanelNative;
				}
			}
		}
	}
#endif
}