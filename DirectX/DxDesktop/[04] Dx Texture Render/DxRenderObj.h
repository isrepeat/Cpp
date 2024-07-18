#pragma once
#include <Helpers/Dx/SwapChainPanel.h>

#include "HDRCommon.h"
#include "StepTimer.h"
#include "Keyboard.h"

#include <mutex>

namespace DxDesktop {
	// To ensure that structures aligned by 16 use dx macros, but may be this not neccessary ...
	XM_ALIGNED_STRUCT(16) VS_CONSTANT_BUFFER {
		DirectX::XMFLOAT4X4 mWorldViewProj;
	};

	XM_ALIGNED_STRUCT(16) PS_CONSTANT_BUFFER {
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
}