#pragma once
#include <Helpers/Dx/SwapChainPanel.h>
#include <Helpers/Dx/FullScreenQuad.h>
#include <Helpers/Dx/DxRenderObjHDR.h>

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
		//QuadRenderer quadRenderer;
		std::unique_ptr<DxRenderObjImage> dxRenderObjImage;
		//std::unique_ptr<DxRenderObjHdrQuad> dxRenderObjHdrQuad;

		H::Dx::details::FullScreenQuad fullScreenQuad;
		std::unique_ptr<H::Dx::details::DxRenderObjHDR> dxRenderObjProxyLocal;
		std::unique_ptr<H::Dx::details::DxRenderObjHDR> dxRenderObjProxy1;
		std::unique_ptr<H::Dx::details::DxRenderObjHDR> dxRenderObjProxy2;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rsState;

		float currentNits; // Current brightness
	};
}