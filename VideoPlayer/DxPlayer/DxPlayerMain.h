#pragma once
#include "Common/Config.h"
#if ENGINE_TYPE == MEDIA_ENDIGNE_LOGIC
#include "Common\StepTimer.h"
#include "Content\VideoSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"
#include "Content\AvReader.h"

// Renders Direct2D and 3D content on the screen.
namespace DxPlayer
{
	class DxPlayerMain
	{
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

#elif ENGINE_TYPE == DX_PLAYER_RENDER

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"
#include "Content\AvReader.h"

// Renders Direct2D and 3D content on the screen.
namespace DxPlayer
{
	class DxPlayerMain : public DX::IDeviceNotify
	{
	public:
		DxPlayerMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~DxPlayerMain();
		void CreateWindowSizeDependentResources();
		void StartTracking() { m_sceneRenderer->StartTracking(); }
		void TrackingUpdate(float positionX) { m_pointerLocationX = positionX; }
		void StopTracking() { m_sceneRenderer->StopTracking(); }
		bool IsTracking() { return m_sceneRenderer->IsTracking(); }
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
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: Replace with your own content renderers.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;

		Windows::Foundation::IAsyncAction^ m_openFileAction;
		Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
		Concurrency::critical_section m_criticalSection;

		// Rendering loop timer.
		DX::StepTimer m_timer;

		// Track current input pointer position.
		float m_pointerLocationX;
	};
}
#endif