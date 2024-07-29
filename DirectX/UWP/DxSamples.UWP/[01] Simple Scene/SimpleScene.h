#pragma once
#include <Helpers/Dx/SwapChainPanel.h>
#include "IScene.h"

namespace DxSamples {
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	class SimpleScene : public IScene {
	public:
		SimpleScene(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel);
		~SimpleScene();

		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();

		void Render() override;

	private:
		std::mutex mx;
		Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel;

		// Direct2D resources
		Microsoft::WRL::ComPtr<ID2D1DrawingStateBlock1> stateBlock;
		Microsoft::WRL::ComPtr<ID2D1Effect> scaleEffect;
		Microsoft::WRL::ComPtr<ID2D1Effect> centerEffect;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
		//Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
		//Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer constantBufferData;

		// Variables used with the rendering loop.
		bool loadingComplete;
	};
}