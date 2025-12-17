#pragma once
#include <Helpers/Dx/SwapChainPanel.h>
#include <Helpers/Com/UnknownValue.h>
#include "DxSettings.h"

namespace Helpers {
	namespace WinRt {
		namespace Dx {
			[Platform::Metadata::Flags]
			public enum class SwapChainPanelInitData_Options : unsigned int {
				_Enum_SwapChainPanelInitData_Options
			};

			public enum class SwapChainPanelInitData_Device {
				DxDevice,
				DxDeviceMF,
				DxVideoDeviceMF,
			};

			public enum class SwapChainPanelInitData_DeviceMutex {
				None,
				Recursive,
			};

			public value struct SwapChainPanelInitData {
				SwapChainPanelInitData_Options optionFlags;
				SwapChainPanelInitData_Device deviceType;
				SwapChainPanelInitData_DeviceMutex deviceMutexType;
			};

			// Controls all the DirectX device resources.
			public ref class SwapChainPanel sealed {
			public:
				SwapChainPanel();
				SwapChainPanel(SwapChainPanelInitData initData);
				SwapChainPanel(SwapChainPanelInitData initData, Helpers::WinRt::Dx::DxSettings^ dxSettings);
				virtual ~SwapChainPanel();

				void SetSwapChainPanelXaml(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanelXaml);
				
				void SetLogicalSize(Windows::Foundation::Size logicalSize);
				Windows::Foundation::Size GetLogicalSize();

				void SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation);
				Windows::Graphics::Display::DisplayOrientations GetCurrentOrientation();

				void SetDpi(float dpi);
				float GetDpi();

				void SetCompositionScale(float compositionScaleX, float compositionScaleY);
				Helpers::WinRt::Scale GetCompositionScale();

				void ValidateDevice();
				void HandleDeviceLost();
				void Trim();
				void Present();

				Helpers::WinRt::Dx::DxSettings^ GetDxSettings();
				Platform::Object^ GetSwapChainPanelNativeAsObject();
				Windows::UI::Xaml::Controls::SwapChainPanel^ GetSwapChainPanelXaml();

			internal:
				void RegisterDeviceNotify(H::Dx::IDeviceNotify* deviceNotify);

			private:
				Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> CreateSwapChainPanelNative(SwapChainPanelInitData initData);
				static void CreateSwapChainPanelDxgi(SwapChainPanel^ _this, IDXGISwapChain3* dxgiSwapChainPanel);
				static H::Rect GetWindowBounds(SwapChainPanel^ _this);

			private:
				Helpers::WinRt::Dx::DxSettings^ dxSettings;
				Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanelNative;
				Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanelXaml;
			};
		}
	}
}