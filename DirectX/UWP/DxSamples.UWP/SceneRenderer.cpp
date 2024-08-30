#include "SceneRenderer.h"
//#include "[01] Simple Scene\SimpleScene.h"
//#include "[02] HDR Scene\HDRScene.h"
#include "[03] Shader Effects\SceneShaderEffects.h"

namespace DxSamples {
	SceneRenderer::SceneRenderer(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanelNative)
		: swapChainPanelNative{ swapChainPanelNative }
	{
		//this->scene = std::make_unique<SimpleScene>(swapChainPanelNative);
		//this->scene = std::make_unique<HDRScene>(swapChainPanelNative);
		this->scene = std::make_unique<SceneShaderEffects>(swapChainPanelNative);

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
	
	void SceneRenderer::OnWindowSizeChanged(H::Size newSize) {
		this->scene->OnWindowSizeChanged(newSize);
	}
}