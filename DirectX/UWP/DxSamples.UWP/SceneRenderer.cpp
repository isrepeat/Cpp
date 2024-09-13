#include "SceneRenderer.h"
#include <Helpers/Time.h>
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
				auto tpStart = std::chrono::high_resolution_clock::now();
				this->scene->Render();
				auto tpA1 = std::chrono::high_resolution_clock::now();
				this->swapChainPanelNative->Present();
				auto tpEnd = std::chrono::high_resolution_clock::now();
				//LOG_DEBUG_D("\n"
				//	"dt [Render] = {}\n"
				//	"dt [Present] = {}\n"
				//	"dt [Render iteration] = {}\n"
				//	, tpA1 - tpStart
				//	, tpEnd - tpA1
				//	, tpEnd - tpStart
				//);
			}
			});
	}

	SceneRenderer::~SceneRenderer() {
		if (this->renderThread.joinable())
			this->renderThread.join();
	}
	
	void SceneRenderer::OnWindowSizeChanged(H::Size newSize) {
		LOG_DEBUG_D("OnWindowSizeChanged(width = {}, height = {})", newSize.width, newSize.height);
		this->scene->OnWindowSizeChanged(newSize);
	}
}