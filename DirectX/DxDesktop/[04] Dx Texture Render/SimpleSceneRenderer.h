#pragma once
#include <Helpers/Dx/SwapChainPanel.h>
#include <mutex>

namespace DxDesktop {
	struct VS_CONSTANT_BUFFER {
		DirectX::XMFLOAT4X4 mWorldViewProj;
	};

	struct DxRenderObj {
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;

		VS_CONSTANT_BUFFER vsConstantBufferData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer;

		void Reset() {
			this->vertexShader.Reset();
			this->pixelShader.Reset();
			this->inputLayout.Reset();
			this->vertexBuffer.Reset();
			this->indexBuffer.Reset();
			this->texture.Reset();
			this->sampler.Reset();
		}

		//operator bool() {
		//	return this->inputLayout;
		//}
	};

	class SimpleSceneRenderer {
	public:
		SimpleSceneRenderer(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel);
		~SimpleSceneRenderer();

		// Basic render loop
		void Tick();

	private:
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();

		DxRenderObj CreateDxRenderObjImage();
		DxRenderObj CreateDxRenderObjTriangle();

		void Clear();
		void Render();
		void RenderImage();
		void RenderTriangle();

	private:
		std::mutex mx;
		Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel;

		// Sample objects
		DxRenderObj dxRenderObjImage;
		DxRenderObj dxRenderObjTriangle;

		float currentPaperWhiteNits; // Current brightness for paper white
		
		// Prepares HDR and SDR swapchain buffers
		Microsoft::WRL::ComPtr<ID3D11PixelShader> d3dPrepareSwapChainBuffersPS; // Outputs an signal for the swapchain buffers to correctly be displayed in HDR/SDR
		Microsoft::WRL::ComPtr<ID3D11PixelShader> d3dTonemapSwapChainBufferPS;
		Microsoft::WRL::ComPtr<ID3D11Buffer> d3dNitsForPaperWhiteCB; // Define the nit value of "paper white", e.g. 100 nits    
	};
}