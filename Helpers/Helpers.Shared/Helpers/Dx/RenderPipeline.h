#pragma once
#include "Helpers/common.h"
#include "DxRenderObjBase.h"
#include "ISwapChainPanel.h"
#include "DxLinkingGraph.h"
#include "DxDevice.h"
#include <memory>

namespace HELPERS_NS {
	namespace Dx {
		struct DxRenderObjQuadGeometryData : DxRenderObjBaseGeometry {
			Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

			void Reset() override {
				this->DxRenderObjBaseGeometry::Reset();
				this->vertexShader.Reset();
				this->inputLayout.Reset();
			}
		};

		class RenderPipeline {
		public:
			RenderPipeline(Microsoft::WRL::ComPtr<HELPERS_NS::Dx::ISwapChainPanel> swapChainPanel);
			RenderPipeline(DxDeviceSafeObj* dxDeviceSafeObj);
			~RenderPipeline() = default;

			void SetTexture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV);
			void SetSampler(Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

			//void AddPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader);
			void SetInputLayout(Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout);
			void SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader, Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer = nullptr);
			void SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader, Microsoft::WRL::ComPtr<ID3D11Buffer> psConstantBuffer = nullptr);

			void SetLinkingGraph(std::shared_ptr<HELPERS_NS::Dx::DxLinkingGraph> dxLinkingGraph);

			void Draw();

		private:
			// Called after SwapChainPanel Presented.
			void ClearState();

		private:
			Microsoft::WRL::ComPtr<HELPERS_NS::Dx::ISwapChainPanel> swapChainPanel;
			DxRenderObjQuadGeometryData quadGeometryData;

			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
			Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;

			//std::vector<Microsoft::WRL::ComPtr<ID3D11PixelShader>> pixelShaders;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
			Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
			Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> psConstantBuffer;

			std::shared_ptr<HELPERS_NS::Dx::DxLinkingGraph> dxLinkingGraph;
		};
	}
}