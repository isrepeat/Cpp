#include "pch.h"
#include "SwapChainPanel.h"
#include "InitDataFactory.h"
#include <Helpers/Dx/DxHelpers.h>
#include <Helpers/CallbackWinRT.hpp>
#include <Helpers/System.h>
#include <Helpers/Action.h>

#include <windows.ui.xaml.media.dxinterop.h>

namespace Tools {
	H::Dx::DisplayOrientations DisplayOrientationToNative(Windows::Graphics::Display::DisplayOrientations displayOrientation) {
		switch (displayOrientation) {
		case Windows::Graphics::Display::DisplayOrientations::Landscape:
			return H::Dx::DisplayOrientations::Landscape;

		case Windows::Graphics::Display::DisplayOrientations::Portrait:
			return H::Dx::DisplayOrientations::Portrait;

		case Windows::Graphics::Display::DisplayOrientations::LandscapeFlipped:
			return H::Dx::DisplayOrientations::LandscapeFlipped;

		case Windows::Graphics::Display::DisplayOrientations::PortraitFlipped:
			return H::Dx::DisplayOrientations::PortraitFlipped;
		}
	}

	Windows::Graphics::Display::DisplayOrientations DisplayOrientationFromNative(H::Dx::DisplayOrientations displayOrientation) {
		switch (displayOrientation) {
		case H::Dx::DisplayOrientations::Landscape:
			return Windows::Graphics::Display::DisplayOrientations::Landscape;

		case H::Dx::DisplayOrientations::Portrait:
			return Windows::Graphics::Display::DisplayOrientations::Portrait;

		case H::Dx::DisplayOrientations::LandscapeFlipped:
			return Windows::Graphics::Display::DisplayOrientations::LandscapeFlipped;

		case H::Dx::DisplayOrientations::PortraitFlipped:
			return Windows::Graphics::Display::DisplayOrientations::PortraitFlipped;
		}
	}
};

namespace Helpers {
	namespace WinRt {
		namespace Dx {
			SwapChainPanel::SwapChainPanel()
				: dxSettings{ nullptr }
				, swapChainPanelNative{ this->CreateSwapChainPanelNative(InitDataFactory::CreateDefaultSwapChainPanelInitData()) }
			{}

			SwapChainPanel::SwapChainPanel(SwapChainPanelInitData initData)
				: dxSettings{ nullptr }
				, swapChainPanelNative{ this->CreateSwapChainPanelNative(initData) }
			{}

			SwapChainPanel::SwapChainPanel(SwapChainPanelInitData initData, Helpers::WinRt::Dx::DxSettings^ dxSettings) 
				: dxSettings{ dxSettings }
				, swapChainPanelNative{ this->CreateSwapChainPanelNative(initData) }
			{}

			SwapChainPanel::~SwapChainPanel() {
			}

			// This method is called when the XAML control is created (or re-created).
			void SwapChainPanel::SetSwapChainPanelXaml(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanelXaml) {
				this->swapChainPanelXaml = swapChainPanelXaml;
				
				H::WinRt::RunOnUIThread(this->swapChainPanelXaml->Dispatcher,
					[this] {
						auto currentDisplayInformation = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
						this->swapChainPanelNative->InitSwapChainPanelInfo(
							H::Size_f{ static_cast<float>(this->swapChainPanelXaml->ActualWidth), static_cast<float>(this->swapChainPanelXaml->ActualHeight) },
							Tools::DisplayOrientationToNative(currentDisplayInformation->NativeOrientation),
							Tools::DisplayOrientationToNative(currentDisplayInformation->CurrentOrientation),
							this->swapChainPanelXaml->CompositionScaleX,
							this->swapChainPanelXaml->CompositionScaleY,
							currentDisplayInformation->LogicalDpi
						);
					});
			}

			// This method is called in the event handler for the SizeChanged event.
			void SwapChainPanel::SetLogicalSize(Windows::Foundation::Size logicalSize) {
				this->swapChainPanelNative->SetLogicalSize(H::Size_f{ logicalSize.Width, logicalSize.Height });
			}

			Windows::Foundation::Size SwapChainPanel::GetLogicalSize() {
				auto size = this->swapChainPanelNative->GetLogicalSize();
				return Windows::Foundation::Size{ size.width, size.height };
			}

			// This method is called in the event handler for the DpiChanged event.
			void SwapChainPanel::SetDpi(float dpi) {
				this->swapChainPanelNative->SetDpi(dpi);
			}

			float SwapChainPanel::GetDpi() {
				return this->swapChainPanelNative->GetDpi();
			}

			// This method is called in the event handler for the OrientationChanged event.
			void SwapChainPanel::SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation) {
				this->swapChainPanelNative->SetCurrentOrientation(Tools::DisplayOrientationToNative(currentOrientation));
			}

			Windows::Graphics::Display::DisplayOrientations SwapChainPanel::GetCurrentOrientation() {
				return Tools::DisplayOrientationFromNative(this->swapChainPanelNative->GetCurrentOrientation());
			}

			// This method is called in the event handler for the CompositionScaleChanged event.
			void SwapChainPanel::SetCompositionScale(float compositionScaleX, float compositionScaleY) {
				this->swapChainPanelNative->SetCompositionScale(compositionScaleX, compositionScaleY);
			}

			Helpers::WinRt::Scale SwapChainPanel::GetCompositionScale() {
				auto scale = this->swapChainPanelNative->GetCompositionScale();
				return Helpers::WinRt::Scale{ scale.x, scale.y };
			}

			// This method is called in the event handler for the DisplayContentsInvalidated event.
			void SwapChainPanel::ValidateDevice() {
				this->swapChainPanelNative->ValidateDevice();
			}

			// Recreate all device resources and set them back to the current state.
			void SwapChainPanel::HandleDeviceLost() {
				assert(false);
				// TODO: implement
			}



			// Call this method when the app suspends. It provides a hint to the driver that the app 
			// is entering an idle state and that temporary buffers can be reclaimed for use by other apps.
			void SwapChainPanel::Trim() {
				this->swapChainPanelNative->Trim();
			}

			// Present the contents of the swap chain to the screen.
			void SwapChainPanel::Present() {
				this->swapChainPanelNative->Present();
			}


			Helpers::WinRt::Dx::DxSettings^ SwapChainPanel::GetDxSettings() {
				return this->dxSettings;
			}

			Platform::Object^ SwapChainPanel::GetSwapChainPanelNativeAsObject() {
				Platform::Object^ obj = reinterpret_cast<Platform::Object^>(this->swapChainPanelNative.Get());
				return obj;
			}

			Windows::UI::Xaml::Controls::SwapChainPanel^ SwapChainPanel::GetSwapChainPanelXaml() {
				return this->swapChainPanelXaml;
			}

			// Register our DeviceNotify to be informed on device lost and creation.
			void SwapChainPanel::RegisterDeviceNotify(H::Dx::IDeviceNotify* deviceNotify) {
				this->swapChainPanelNative->RegisterDeviceNotify(deviceNotify);
			}

			Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> SwapChainPanel::CreateSwapChainPanelNative(SwapChainPanelInitData initData) {
				H::Dx::SwapChainPanel::InitData initDataNative;
				initDataNative.environment = H::Dx::SwapChainPanel::InitData::Environment::UWP;
				initDataNative.fnCreateSwapChainPannelDxgi = H::MakeWinRTCallback(this, &SwapChainPanel::CreateSwapChainPanelDxgi);
				initDataNative.fnGetWindowBounds = H::MakeWinRTCallback(this, &SwapChainPanel::GetWindowBounds);
				initDataNative.optionFlags = static_cast<H::Dx::SwapChainPanel::InitData::Options>(initData.optionFlags);

				if (initDataNative.optionFlags.Has(H::Dx::SwapChainPanel::InitData::Options::EnableHDR)) {
					initDataNative.backBufferFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
				}

				switch (initData.deviceType) {
				case SwapChainPanelInitData_Device::DxDevice:
					initDataNative.dxDeviceFactory = [] {
						return std::make_unique<H::Dx::details::DxDevice>();
					};
					break;
				case SwapChainPanelInitData_Device::DxDeviceMF:
					initDataNative.dxDeviceFactory = [] {
						return std::make_unique<H::Dx::details::DxDeviceMF>();
					};
					break;
				case SwapChainPanelInitData_Device::DxVideoDeviceMF:
					initDataNative.dxDeviceFactory = [] {
						return std::make_unique<H::Dx::details::DxVideoDeviceMF>();
					};
					break;
				}

				switch (initData.deviceMutexType) {
				case SwapChainPanelInitData_DeviceMutex::None:
					initDataNative.dxDeviceSafeObjMutexFactory = [] {
						return std::make_unique<H::EmptyMutex>();
					};
					break;
				case SwapChainPanelInitData_DeviceMutex::Recursive:
					initDataNative.dxDeviceSafeObjMutexFactory = [] {
						return std::make_unique<H::Mutex<std::recursive_mutex>>();
					};
					break;
				}

				if (this->dxSettings) {
					initDataNative.dxSettingsWeak = this->dxSettings->GetDxSettingsNative();
				}

				return Microsoft::WRL::Make<H::Dx::SwapChainPanel>(initDataNative);
			}


			void SwapChainPanel::CreateSwapChainPanelDxgi(SwapChainPanel^ _this, IDXGISwapChain3* swapChainPanelDxgi) {
				// Associate swap chain with SwapChainPanel
				// UI changes will need to be dispatched back to the UI thread
				_this->swapChainPanelXaml->Dispatcher->RunAsync(
					Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([=] {
						HRESULT hr = S_OK;

						// Get backing native interface for SwapChainPanel
						Microsoft::WRL::ComPtr<ISwapChainPanelNative> swapChainPanelNative;
						hr = reinterpret_cast<IUnknown*>(_this->swapChainPanelXaml)->QueryInterface(IID_PPV_ARGS(swapChainPanelNative.GetAddressOf()));
						H::System::ThrowIfFailed(hr);

						hr = swapChainPanelNative->SetSwapChain(swapChainPanelDxgi);
						H::System::ThrowIfFailed(hr);
						},
						Platform::CallbackContext::Any));
			}

			H::Rect  SwapChainPanel::GetWindowBounds(SwapChainPanel^ _this) {
				auto rect = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->VisibleBounds;
				return static_cast<H::Rect>(H::Rect_f{ rect.Left, rect.Top, rect.Right, rect.Bottom });
			}
		}
	}
}