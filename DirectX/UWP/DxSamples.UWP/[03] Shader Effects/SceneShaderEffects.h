#pragma once
#include <Helpers/Dx/SwapChainPanel.h>
#include <Helpers/Dx/RenderPipeline.h>
#include <Helpers/Dx/DxLinkingGraph.h>

#include "IScene.h"
#include "DxRenderObjImage.h"
#include "StepTimer.h"

#include <mutex>

// https://stackoverflow.com/questions/41210159/directx-shader-linking-error
// https://learn.microsoft.com/ru-ru/windows/win32/api/d3d11shader/nn-d3d11shader-id3d11functionlinkinggraph
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/constructing-a-function-linking-graph
// https://github.com/MicrosoftDocs/win32/blob/docs/desktop-src/direct3dhlsl/constructing-a-function-linking-graph.md
// https://qiita.com/ryutorion/items/6dc733265a7d7c5f5ef6


struct VS_CONSTANT_BUFFER_MUDULE_A {
	DirectX::XMFLOAT4X4 mWorldViewProj;
};


namespace DxSamples {
	class SceneShaderEffects : public IScene {
	public:
		SceneShaderEffects(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel);
		~SceneShaderEffects();

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

		void RenderScene();

	private:
		std::mutex mx;
		Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel;
		std::shared_ptr<H::Dx::DxLinkingGraph> dxLinkingGraph;
		H::Dx::RenderPipeline renderPipeline;

		std::unique_ptr<DxRenderObjImage> dxRenderImageObj;
		std::vector<std::unique_ptr<H::Dx::details::DxRenderObjProxy>> dxRenderProxyObjects;

		// Rendering loop timer.
		DX::StepTimer renderLoopTimer;

		H::Dx::HlslModule hlslVertexModule1;
		H::Dx::HlslModule hlslVertexModule2;

		H::Dx::HlslModule hlslPixelModule1;
		H::Dx::HlslModule hlslPixelModule2;
	};
}