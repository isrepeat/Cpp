#pragma once
#include <Helpers/Dx/SwapChainPanel.h>

#include "IScene.h"
#include "DxRenderObjHdrQuad.h"
#include "DxRenderObjImage.h"
#include "DxRenderObj.h"
#include "QuadRenderer.h"
#include "HDRCommon.h"
#include "StepTimer.h"

#include <mutex>

namespace DxSamples {
	class HDRScene : public IScene {
	public:
		HDRScene(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel);
		~HDRScene();

		// Basic render loop
		void Render() override;
		void OnWindowSizeChanged(H::Size newSize) override;

	private:
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();

		void Update();
		void Clear();
		void RenderInternal();

		void RenderHDRScene();

	private:
		std::mutex mx;
		Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel;

		// Rendering loop timer.
		DX::StepTimer renderLoopTimer;

		// Sample objects.
		QuadRenderer quadRenderer;
		std::unique_ptr<DxRenderObjImage> dxRenderObjImage;
		std::unique_ptr<DxRenderObjHdrQuad> dxRenderObjHdrQuad;

		float currentNits; // Current brightness
	};
}