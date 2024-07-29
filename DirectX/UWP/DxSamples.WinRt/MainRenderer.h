#pragma once
#include <SceneRenderer.h>
#include <concrt.h>

namespace DxSamples {
    namespace WinRt {
        public ref class MainRenderer sealed {
        public:
            MainRenderer(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanelXaml);

        private:
            Helpers::WinRt::Dx::SwapChainPanel^ CreateSwapChainPanelWinRt(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanelXaml);

            // Event handlers
            void OnSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);

        private:
            Concurrency::critical_section criticalSection;

            Helpers::WinRt::Dx::SwapChainPanel^ swapChainPanelWinRt;
            Microsoft::WRL::ComPtr<HELPERS_NS::Dx::ISwapChainPanel> swapChainPanelNative;

            DxSamples::SceneRenderer sceneRenderer;
        };
    }
}
