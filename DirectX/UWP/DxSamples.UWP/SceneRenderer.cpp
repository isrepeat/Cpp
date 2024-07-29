#include "SceneRenderer.h"
#include "[01] Simple Scene\SimpleScene.h"

namespace DxSamples {
	SceneRenderer::SceneRenderer(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanelNative)
		: swapChainPanelNative{ swapChainPanelNative }
	{
		this->scene = std::make_unique<SimpleScene>(swapChainPanelNative);

		this->renderThread = std::thread([&] {
			while (true /*isRenderRunning*/) {
				this->scene->Render();
				this->swapChainPanelNative->Present();
			}
			});
	}

	SceneRenderer::~SceneRenderer() {
		if (this->renderThread.joinable())
			this->renderThread.join();
	}
}