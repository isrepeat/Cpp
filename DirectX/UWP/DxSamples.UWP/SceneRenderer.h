#pragma once
#include <Helpers/Dx/SwapChainPanel.h>
#include "IScene.h"
#include <thread>
#include <memory>

namespace DxSamples {
	class SceneRenderer {
	public:
		SceneRenderer(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanelNative);
		~SceneRenderer();

		void OnWindowSizeChanged(H::Size newSize);

	private:
		Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanelNative;
		std::unique_ptr<IScene> scene;
		std::thread renderThread;
	};
}