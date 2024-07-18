#pragma once
#include <Helpers/Dx/SwapChainPanel.h>

#include "DxRenderObj.h"
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
		DxRenderObjHDR CreateDxRenderObjHdrQuad();
		void CreateHdrTexture(DxRenderObjBase* dxRenderObjBase);

		void Update();
		void Clear();
		void Render();

		void RenderHDRScene();
		void PrepareSwapChainBuffer();

	private:
		std::mutex mx;
		Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel;

		// Rendering loop timer.
		DX::StepTimer renderLoopTimer;

		// Input devices.
		std::unique_ptr<DirectX::Keyboard> keyboard;
		DirectX::Keyboard::KeyboardStateTracker keyboardButtons;

		// Sample objects.
		DxRenderObj dxRenderObjImage;
		DxRenderObjHDR dxRenderObjHdrQuad;

		float currentNits; // Current brightness
		
		// Prepares HDR and SDR swapchain buffers
		Microsoft::WRL::ComPtr<ID3D11PixelShader> d3dPrepareSwapChainBuffersPS; // Outputs an signal for the swapchain buffers to correctly be displayed in HDR/SDR
		Microsoft::WRL::ComPtr<ID3D11PixelShader> d3dTonemapSwapChainBufferPS;
		Microsoft::WRL::ComPtr<ID3D11Buffer> d3dNitsForPaperWhiteCB; // Define the nit value of "paper white", e.g. 100 nits    
	};
}