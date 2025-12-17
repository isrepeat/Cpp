#pragma once
#include "Helpers/common.h"
#include "DxDevice.h"
#include <d3dcompiler.h>

namespace HELPERS_NS {
	namespace Dx {
		class DxConstantBufferBase {
		public:
			DxConstantBufferBase(HELPERS_NS::Dx::DxDeviceSafeObj* dxDeviceSafeObj, uint32_t bufferByteWidth)
				: dxDeviceSafeObj{ dxDeviceSafeObj }
			{
				D3D11_BUFFER_DESC constantbufferDesc;
				constantbufferDesc.ByteWidth = bufferByteWidth;
				constantbufferDesc.Usage = D3D11_USAGE_DEFAULT;
				constantbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				constantbufferDesc.CPUAccessFlags = 0;
				constantbufferDesc.MiscFlags = 0;
				constantbufferDesc.StructureByteStride = 0;

				auto dxDev = this->dxDeviceSafeObj->Lock();
				auto d3dDev = dxDev->GetD3DDevice();

				HRESULT hr = S_OK;
				hr = d3dDev->CreateBuffer(
					&constantbufferDesc,
					nullptr,
					this->constantBuffer.ReleaseAndGetAddressOf()
				);
				HELPERS_NS::System::ThrowIfFailed(hr);
			}
			virtual ~DxConstantBufferBase() = default;
			virtual void UpdateSubresource(Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dCtx = nullptr) = 0;

			Microsoft::WRL::ComPtr<ID3D11Buffer> GetBuffer() const {
				return this->constantBuffer;
			}

		protected:
			HELPERS_NS::Dx::DxDeviceSafeObj* dxDeviceSafeObj;
			Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
		};

		template<typename T>
		struct DxConstantBuffer : public DxConstantBufferBase {
			T constantBufferData;

			DxConstantBuffer(HELPERS_NS::Dx::DxDeviceSafeObj* dxDeviceSafeObj)
				: DxConstantBufferBase(dxDeviceSafeObj, sizeof(T))
			{}

			void UpdateSubresource(Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dCtx) override {
				if (d3dCtx) {
					d3dCtx->UpdateSubresource(this->constantBuffer.Get(), 0, nullptr, &this->constantBufferData, 0, 0);
				}
				else {
					auto dxDev = this->dxDeviceSafeObj->Lock();
					auto d3dDev = dxDev->GetD3DDevice();
					auto dxCtx = dxDev->LockContext();
					auto d3dCtx = dxCtx->D3D();
					d3dCtx->UpdateSubresource(this->constantBuffer.Get(), 0, nullptr, &this->constantBufferData, 0, 0);
				}
			}
		};
	}
}