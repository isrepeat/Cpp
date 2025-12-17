#pragma once
#include <Helpers/common.h>
#include <Helpers/System.h>
#include <Helpers/Math.h>
#include "DxIncludes.h"
#include <optional>
#include <memory>

namespace HELPERS_NS {
	namespace Dx {
		struct VertexPositionTexcoord {
			DirectX::XMFLOAT4 position;
			DirectX::XMFLOAT2 texcoord;
		};
		struct VertexPositionColor {
			DirectX::XMFLOAT4 position;
			DirectX::XMFLOAT4 color;
		};
		

		struct IDxRenderObj {
			virtual void Reset() = 0;
		};

		struct DxRenderObjBaseGeometry : IDxRenderObj {
			Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

			void Reset() override {
				this->vertexBuffer.Reset();
				this->indexBuffer.Reset();
			}
		};


		struct DxTextureResources {
			Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> textureRTV;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
			Microsoft::WRL::ComPtr<ID2D1Bitmap1> bitmap;
		};

		// TODO: use vector of DxTextureResources.
		struct DxRenderObjBaseTexture : IDxRenderObj {
			Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> textureRTV;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
			Microsoft::WRL::ComPtr<ID2D1Bitmap1> bitmap;
			Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;

			void Reset() override {
				this->texture.Reset();
				this->textureRTV.Reset();
				this->textureSRV.Reset();
				this->bitmap.Reset();
				this->sampler.Reset();
			}
		};


		struct DxRenderObjBaseData : DxRenderObjBaseGeometry, DxRenderObjBaseTexture {
			void Reset() override {
				this->DxRenderObjBaseGeometry::Reset();
				this->DxRenderObjBaseTexture::Reset();
			}
		};

		struct DxRenderObjDefaultData : DxRenderObjBaseData {
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
			Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

			void Reset() override {
				this->DxRenderObjBaseData::Reset();
				this->vsConstantBuffer.Reset();
				this->psConstantBuffer.Reset();
				this->vertexShader.Reset();
				this->inputLayout.Reset();
				this->pixelShader.Reset();
			}
		};


		template <typename DxRenderObjDataT, typename... Args>
		class DxRenderObjWrapper {
		public:
			virtual void CreateWindowSizeDependentResources(std::optional<HELPERS_NS::Size> size = std::nullopt) = 0;
			virtual void ReleaseDeviceDependentResources() = 0;
			virtual void UpdateBuffers() = 0;

			std::unique_ptr<DxRenderObjDataT, Args...>& operator->() {
				return this->dxRenderObjData;
			}

			std::unique_ptr<DxRenderObjDataT, Args...>& GetObjData() {
				return this->dxRenderObjData;
			}

		protected:
			std::unique_ptr<DxRenderObjDataT, Args...> dxRenderObjData;
		};


		inline Microsoft::WRL::ComPtr<ID3D11SamplerState> CreateLinearSampler(Microsoft::WRL::ComPtr<ID3D11Device> d3dDev) {
			HRESULT hr = S_OK;

			D3D11_SAMPLER_DESC samplerDesc = {};
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

			samplerDesc.MinLOD = 0;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			Microsoft::WRL::ComPtr<ID3D11SamplerState> linearSampler;
			hr = d3dDev->CreateSamplerState(&samplerDesc, linearSampler.ReleaseAndGetAddressOf());
			HELPERS_NS::System::ThrowIfFailed(hr);

			return linearSampler;
		}
	}
}