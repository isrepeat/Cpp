#include "MainRenderer.h"

namespace DxSamples {
    namespace WinRt {
        MainRenderer::MainRenderer(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanelXaml)
            : swapChainPanelWinRt{ this->CreateSwapChainPanelWinRt(swapChainPanelXaml) }
            , swapChainPanelNative{ HELPERS_NS::Dx::WinRt::Tools::QuerySwapChainPanelNative(this->swapChainPanelWinRt->GetSwapChainPanelNativeAsObject()) }
            , sceneRenderer{ this->swapChainPanelNative }
        {
        }

        Helpers::WinRt::Dx::SwapChainPanel^ MainRenderer::CreateSwapChainPanelWinRt(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanelXaml) {
            Helpers::WinRt::Dx::SwapChainPanelInitData initData;
            initData.deviceType = Helpers::WinRt::Dx::SwapChainPanelInitData_Device::DxDevice;
            initData.deviceMutexType = Helpers::WinRt::Dx::SwapChainPanelInitData_DeviceMutex::Recursive;
            initData.optionFlags = Helpers::WinRt::Dx::SwapChainPanelInitData_Options::EnableHDR;

            auto swapChainPanelWinRt = ref new Helpers::WinRt::Dx::SwapChainPanel(initData);
            swapChainPanelWinRt->SetSwapChainPanelXaml(swapChainPanelXaml);

            // Setup control / application event handlers
            swapChainPanelXaml->SizeChanged += ref new Windows::UI::Xaml::SizeChangedEventHandler(
                this, &MainRenderer::OnSizeChanged
            );
            swapChainPanelXaml->CompositionScaleChanged += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Xaml::Controls::SwapChainPanel^, Object^>(
                this, &MainRenderer::OnCompositionScaleChanged
            );
            return swapChainPanelWinRt;
        }

        void MainRenderer::OnSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e) {
            Concurrency::critical_section::scoped_lock lock{ criticalSection };

            //Windows.UI.ViewManagement.ApplicationView.GetForCurrentView().VisibleBounds;
            
            //Windows::Graphics::Display::DisplayInformation::GetForCurrentView()

            auto size = e->NewSize;
            auto displayInformation = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
            
            this->swapChainPanelWinRt->SetLogicalSize(e->NewSize);
            this->sceneRenderer.OnWindowSizeChanged(static_cast<H::Size>(H::Size_f{ size.Width, size.Height }));
        }

        void MainRenderer::OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Platform::Object^ args) {
            //this->swapChainPanelWinRt->SetCompositionScale(e->NewSize);
        }
    }
}