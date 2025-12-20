#pragma once
#include "Sources\StepTimer.h"
#include "Sources\VideoSceneRenderer.h"
#include "Sources\AvReader.h"

// Renders Direct2D and 3D content on the screen.
namespace TestVideoPlayer {
    class DxPlayerMain {
    public:
        DxPlayerMain(Helpers::WinRt::Dx::SwapChainPanel^ swapChainPanelWinRt);
        ~DxPlayerMain();
        void CreateWindowSizeDependentResources();
        void StartRenderLoop();
        void StopRenderLoop();
        Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

        // IDeviceNotify
        virtual void OnDeviceLost();
        virtual void OnDeviceRestored();

    private:
        void ProcessInput();
        void Update();
        bool Render();

        // Cached pointer to device resources.
        Helpers::WinRt::Dx::SwapChainPanel^ swapChainPanelWinRt;
        Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanelNative;

        // TODO: Replace with your own content renderers.
        std::unique_ptr<VideoSceneRenderer> m_sceneRenderer;

        Windows::Foundation::IAsyncAction^ m_openFileAction;
        Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
        Concurrency::critical_section m_criticalSection;

        // Rendering loop timer.
        DX::StepTimer m_timer;

        // Track current input pointer position.
        float m_pointerLocationX;
    };
}