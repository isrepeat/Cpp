#pragma once
#include <Helpers/Dx/DxRenderObjBase.h>
#include <Helpers/Dx/ISwapChainPanel.h>

namespace DxSamples {
	struct DxRenderObjImageData : H::Dx::DxRenderObjBaseTexture {
		struct VS_CONSTANT_BUFFER {
			DirectX::XMFLOAT4X4 mWorldViewProj;
		};
		struct PS_CONSTANT_BUFFER {
			DirectX::XMFLOAT4 someData;
		};

		VS_CONSTANT_BUFFER vsConstantBufferData;
		PS_CONSTANT_BUFFER psConstantBufferData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> psConstantBuffer;
		
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

		void Reset() override {
			this->DxRenderObjBaseTexture::Reset();
			this->vsConstantBuffer.Reset();
			this->psConstantBuffer.Reset();
			this->vertexShader.Reset();
			this->pixelShader.Reset();
		}
	};

	class DxRenderObjImage : public H::Dx::DxRenderObjWrapper<DxRenderObjImageData> {
	public:
		// Shader filepathes are relative to .exe
		struct Params {
			std::filesystem::path imagePath;
			std::filesystem::path vertexShader;
			std::filesystem::path pixelShader;
		};

		DxRenderObjImage(
			Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel,
			Params params);

		void CreateWindowSizeDependentResources() override;
		void ReleaseDeviceDependentResources() override;
		void UpdateBuffers() override;

	private:
		std::unique_ptr<DxRenderObjImageData> CreateObjectData(
			H::Dx::DxDeviceSafeObj* dxDeviceSafeObj,
			Params params);

	private:
		Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel;
	};
}