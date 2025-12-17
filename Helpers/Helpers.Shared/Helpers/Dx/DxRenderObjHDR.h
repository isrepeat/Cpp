#pragma once
#include <Helpers/common.h>
#include "DxRenderObjBase.h"
#include "ISwapChainPanel.h"
#include "DxDevice.h"

namespace HELPERS_NS {
	namespace Dx {
		namespace details {
			struct DxRenderObjHDRData : DxRenderObjBaseData {
				struct VS_CONSTANT_BUFFER {
					DirectX::XMFLOAT4X4 mWorldViewProj;
				};
				struct PS_CONSTANT_BUFFER {
					DirectX::XMFLOAT4 luminance;
				};

				VS_CONSTANT_BUFFER vsConstantBufferData;
				PS_CONSTANT_BUFFER psConstantBufferData;
				Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer;
				Microsoft::WRL::ComPtr<ID3D11Buffer> psConstantBuffer;

				Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
				Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
				Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShaderHDR;
				Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShaderToneMap;

				void Reset() override {
					this->DxRenderObjBaseData::Reset();
					this->vsConstantBuffer.Reset();
					this->psConstantBuffer.Reset();
					this->vertexShader.Reset();
					this->inputLayout.Reset();
					this->pixelShaderHDR.Reset();
					this->pixelShaderToneMap.Reset();
				}
			};
			
			class DxRenderObjHDR : public DxRenderObjWrapper<DxRenderObjHDRData> {
			public:
				struct Events {
					HELPERS_NS::Event::Signal<void(HELPERS_NS::Size)> additionalSizeDependentHandlers;
				};

				// Shader filepathes are relative to .exe
				struct Params {
					std::filesystem::path vertexShader;
					std::filesystem::path pixelShaderHDR;
					std::filesystem::path pixelShaderToneMap;
				};

				DxRenderObjHDR(
					Microsoft::WRL::ComPtr<HELPERS_NS::Dx::ISwapChainPanel> swapChainPanel,
					Params params
				);

				DxRenderObjHDR(
					DxDeviceSafeObj* dxDeviceSafeObj,
					Params params
				);

				//
				// DxRenderObjWrapper
				//
				void CreateWindowSizeDependentResources(std::optional<HELPERS_NS::Size> size = std::nullopt) override;
				void ReleaseDeviceDependentResources() override;
				void UpdateBuffers() override;

				//
				// API
				//
				const Events& GetEvents() const;
				DxTextureResources GetTextureResources();

			private:
				std::unique_ptr<DxRenderObjHDRData> CreateObjectData(
					HELPERS_NS::Dx::DxDeviceSafeObj* dxDeviceSafeObj,
					Params params);

				void CreateTexture(HELPERS_NS::Size size);

			private:
				Events events;
				Microsoft::WRL::ComPtr<HELPERS_NS::Dx::ISwapChainPanel> swapChainPanel;
			};
		}
	}
}