#pragma once
#include <Helpers/Dx/SwapChainPanel.h>

#include "HDRCommon.h"
#include "StepTimer.h"
#include "Tools.h"

#include <mutex>

namespace DxSamples {
	struct VertexPositionTexcoord {
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT2 texcoord;
	};
	struct VertexPositionColor {
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 color;
	};

	struct VS_CONSTANT_BUFFER {
		DirectX::XMFLOAT4X4 mWorldViewProj;
	};

	struct PS_CONSTANT_BUFFER {
		DirectX::XMFLOAT4 currentNits;
	};


	struct DxRenderObjBase {
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> textureRTV;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;

		VS_CONSTANT_BUFFER vsConstantBufferData;
		PS_CONSTANT_BUFFER psConstantBufferData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> psConstantBuffer;

		virtual void Reset() {
			this->vertexShader.Reset();
			this->inputLayout.Reset();
			this->vertexBuffer.Reset();
			this->indexBuffer.Reset();
			this->texture.Reset();
			this->textureRTV.Reset();
			this->textureSRV.Reset();
			this->sampler.Reset();
			this->vsConstantBuffer.Reset();
			this->psConstantBuffer.Reset();
		}
	};


	struct DxRenderObj : DxRenderObjBase {
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

		void Reset() override {
			this->DxRenderObjBase::Reset();
			this->pixelShader.Reset();
		}
	};

	struct DxRenderObjHDR : DxRenderObjBase {
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShaderHDR;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShaderToneMap;

		void Reset() override {
			this->DxRenderObjBase::Reset();
			this->pixelShaderHDR.Reset();
			this->pixelShaderToneMap.Reset();
		}
	};


	template <typename DxRenderObjT, typename... Args>
	class DxRenderObjProvider {
	public:
		virtual void CreateWindowSizeDependentResources() = 0;
		virtual void ReleaseDeviceDependentResources() = 0;

		std::unique_ptr<DxRenderObjT, Args...>& operator->() {
			return this->dxRenderObj;
		}

		std::unique_ptr<DxRenderObjT, Args...>& GetObj() {
			return this->dxRenderObj;
		}

	protected:
		std::unique_ptr<DxRenderObjT, Args...> dxRenderObj;
	};
}