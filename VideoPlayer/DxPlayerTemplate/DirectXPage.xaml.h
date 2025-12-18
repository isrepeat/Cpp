#pragma once
#include "DirectXPage.g.h"

#include "Common/DeviceResources.h"
#include "DxPlayerTemplateMain.h"

namespace DxPlayerTemplate
{
	public ref class DirectXPage sealed
	{
	public:
		DirectXPage();
		virtual ~DirectXPage();

		void SaveInternalState(Windows::Foundation::Collections::IPropertySet^ state);
		void LoadInternalState(Windows::Foundation::Collections::IPropertySet^ state);

	private:
		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
		void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Platform::Object^ args);
		void OnSwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
		void OnRendering(Platform::Object^ sender, Platform::Object^ args);

		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		std::unique_ptr<DxPlayerTemplateMain> m_main;
		bool m_windowVisible;
		Windows::Foundation::EventRegistrationToken m_renderingEventToken;
	};
}
