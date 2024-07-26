#pragma once
#include <Helpers/Dx/SwapChainPanel.h>

#include "QuadRenderer.h"
#include "DxRenderObj.h"
#include "DxRenderObjHdrQuad.h"
#include "DxRenderObjProxy.h"
#include "DxRenderObjImage.h"
#include "HDRCommon.h"
#include "StepTimer.h"
#include "Keyboard.h"

#include <mutex>

namespace DxDesktop {
	class SimpleSceneRenderer {
	public:
		SimpleSceneRenderer(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel);
		~SimpleSceneRenderer();

		// Basic render loop
		void Tick();

		void OnWindowSizeChanged(H::Size newSize);

	private:
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();

		DxRenderObj CreateDxRenderObjImage();
		DxRenderObj CreateDxRenderObjProxy();
		DxRenderObjHDR CreateDxRenderObjHdrQuad();
		void CreateProxyTexture(DxRenderObjBase* dxRenderObjBase);
		void CreateHdrTexture(DxRenderObjBase* dxRenderObjBase);

		void Update();
		void Clear();
		void Render();

		void RenderHDRScene();

	private:
		std::mutex mx;
		Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel;

		// Rendering loop timer.
		DX::StepTimer renderLoopTimer;

		// Input devices.
		std::unique_ptr<DirectX::Keyboard> keyboard;
		DirectX::Keyboard::KeyboardStateTracker keyboardButtons;

		// Sample objects.
		QuadRenderer quadRenderer;
		std::unique_ptr<DxRenderObjImage> dxRenderObjImage;
		std::unique_ptr<DxRenderObjProxy> dxRenderObjProxy;
		std::unique_ptr<DxRenderObjHdrQuad> dxRenderObjHdrQuad;

		float currentNits; // Current brightness
		
		// Prepares HDR and SDR swapchain buffers
		Microsoft::WRL::ComPtr<ID3D11PixelShader> d3dPrepareSwapChainBuffersPS; // Outputs an signal for the swapchain buffers to correctly be displayed in HDR/SDR
		Microsoft::WRL::ComPtr<ID3D11PixelShader> d3dTonemapSwapChainBufferPS;
		Microsoft::WRL::ComPtr<ID3D11Buffer> d3dNitsForPaperWhiteCB; // Define the nit value of "paper white", e.g. 100 nits    
	};
}