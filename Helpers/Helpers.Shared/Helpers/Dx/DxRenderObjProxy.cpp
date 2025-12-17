#include "DxRenderObjProxy.h"
#include <Helpers/Dx/Shaders/ShadersCommon.h>
#include <Helpers/FileSystem.h>

namespace HELPERS_NS {
    namespace Dx {
		namespace details {
			DxRenderObjProxy::DxRenderObjProxy(
				Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel,
				DXGI_FORMAT textureFormat)
				: swapChainPanel{ swapChainPanel }
				, textureFormat{ textureFormat }
			{
				this->dxRenderObjData = this->CreateObjectData(this->swapChainPanel->GetDxDevice());
				this->CreateWindowSizeDependentResources();
			}

			void DxRenderObjProxy::CreateWindowSizeDependentResources(std::optional<H::Size> size) {
				H::Size newSize;
				if (size) {
					newSize = size.value();
				}
				else {
					newSize = static_cast<H::Size>(this->swapChainPanel->GetOutputSize());
				}
				this->CreateTexture(newSize);
			}

			void DxRenderObjProxy::ReleaseDeviceDependentResources() {
				this->dxRenderObjData->Reset();
			}

			void DxRenderObjProxy::UpdateBuffers() {
				auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
				auto d3dDev = dxDev->GetD3DDevice();
				auto dxCtx = dxDev->LockContext();
				auto d3dCtx = dxCtx->D3D();

				d3dCtx->UpdateSubresource(this->dxRenderObjData->vsConstantBuffer.Get(), 0, nullptr, &this->dxRenderObjData->vsConstantBufferData, 0, 0);
				d3dCtx->UpdateSubresource(this->dxRenderObjData->psConstantBuffer.Get(), 0, nullptr, &this->dxRenderObjData->psConstantBufferData, 0, 0);
			}

			std::unique_ptr<DxRenderObjDefaultData> DxRenderObjProxy::CreateObjectData(DxDeviceSafeObj* dxDeviceSafeObj) {
				HRESULT hr = S_OK;

				auto dxDev = dxDeviceSafeObj->Lock();
				auto d3dDev = dxDev->GetD3DDevice();
				auto dxCtx = dxDev->LockContext();
				auto d2dCtx = dxCtx->D2D();

				auto dxRenderObjData = std::make_unique<DxRenderObjDefaultData>();

				// Create vertex buffer.
				{
					float w = 1.0f;
					float h = 1.0f;
					static const VertexPositionTexcoord vertexData[4] = {
						{ { -w, -h, 0.1f, 1.0f }, { 0.0f, 1.0f } }, // LB
						{ { -w,  h, 0.1f, 1.0f }, { 0.0f, 0.0f } }, // LT
						{ {  w,  h, 0.1f, 1.0f }, { 1.0f, 0.0f } }, // RT
						{ {  w, -h, 0.1f, 1.0f }, { 1.0f, 1.0f } }, // RB
					};

					D3D11_SUBRESOURCE_DATA initData = {};
					initData.pSysMem = vertexData;

					D3D11_BUFFER_DESC vertexBufferDesc = {};
					vertexBufferDesc.ByteWidth = sizeof(vertexData);
					vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
					vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
					vertexBufferDesc.StructureByteStride = sizeof(VertexPositionTexcoord);

					hr = d3dDev->CreateBuffer(
						&vertexBufferDesc,
						&initData,
						dxRenderObjData->vertexBuffer.ReleaseAndGetAddressOf()
					);
					H::System::ThrowIfFailed(hr);
				}

				// Create index buffer.
				{
					static const uint16_t indexData[6] = {
						0,1,2,
						2,3,0,
					};

					D3D11_SUBRESOURCE_DATA initData = {};
					initData.pSysMem = indexData;

					D3D11_BUFFER_DESC indexBufferDesc = {};
					indexBufferDesc.ByteWidth = sizeof(indexData);
					indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
					indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
					indexBufferDesc.StructureByteStride = sizeof(uint16_t);

					hr = d3dDev->CreateBuffer(
						&indexBufferDesc,
						&initData,
						dxRenderObjData->indexBuffer.ReleaseAndGetAddressOf()
					);
					H::System::ThrowIfFailed(hr);
				}

				// Create sampler.
				{
					dxRenderObjData->sampler = H::Dx::CreateLinearSampler(d3dDev);
				}

				// Load vertex shader and create input layout.
				{
					auto vertexShaderBlob = H::FS::ReadFile(g_shaderLoadDir / L"defaultVS.cso");
					hr = d3dDev->CreateVertexShader(
						vertexShaderBlob.data(),
						vertexShaderBlob.size(),
						nullptr,
						dxRenderObjData->vertexShader.ReleaseAndGetAddressOf()
					);
					H::System::ThrowIfFailed(hr);

					static const D3D11_INPUT_ELEMENT_DESC inputElementDesc[2] = {
						{ "SV_Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA,  0 },
						{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA , 0 },
					};

					hr = d3dDev->CreateInputLayout(
						inputElementDesc,
						_countof(inputElementDesc),
						vertexShaderBlob.data(),
						vertexShaderBlob.size(),
						dxRenderObjData->inputLayout.ReleaseAndGetAddressOf()
					);
					H::System::ThrowIfFailed(hr);
				}

				// Load pixel shaders.
				{
					auto pixelShaderBlob = H::FS::ReadFile(g_shaderLoadDir / L"defaultPS.cso");
					hr = d3dDev->CreatePixelShader(
						pixelShaderBlob.data(),
						pixelShaderBlob.size(),
						nullptr,
						dxRenderObjData->pixelShader.ReleaseAndGetAddressOf()
					);
					H::System::ThrowIfFailed(hr);
				}

				// VS constant buffer.
				{
					DirectX::XMStoreFloat4x4(
						&dxRenderObjData->vsConstantBufferData.mWorldViewProj,
						DirectX::XMMatrixTranspose(
							DirectX::XMMatrixIdentity()
						)
					);

					D3D11_BUFFER_DESC constantbufferDesc;
					constantbufferDesc.ByteWidth = sizeof(DxRenderObjDefaultData::VS_CONSTANT_BUFFER);
					constantbufferDesc.Usage = D3D11_USAGE_DEFAULT;
					constantbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
					constantbufferDesc.CPUAccessFlags = 0;
					constantbufferDesc.MiscFlags = 0;
					constantbufferDesc.StructureByteStride = 0;

					hr = d3dDev->CreateBuffer(
						&constantbufferDesc,
						nullptr,
						dxRenderObjData->vsConstantBuffer.ReleaseAndGetAddressOf()
					);
					H::System::ThrowIfFailed(hr);
				}

				// PS constant buffer.
				{
					// Store default values
					dxRenderObjData->psConstantBufferData.someData = { 0.0f, 0.0f, 0.0f, 0.0f };

					D3D11_BUFFER_DESC constantbufferDesc;
					constantbufferDesc.ByteWidth = sizeof(DxRenderObjDefaultData::PS_CONSTANT_BUFFER);
					constantbufferDesc.Usage = D3D11_USAGE_DEFAULT;
					constantbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
					constantbufferDesc.CPUAccessFlags = 0;
					constantbufferDesc.MiscFlags = 0;
					constantbufferDesc.StructureByteStride = 0;

					hr = d3dDev->CreateBuffer(
						&constantbufferDesc,
						nullptr,
						dxRenderObjData->psConstantBuffer.ReleaseAndGetAddressOf()
					);
					H::System::ThrowIfFailed(hr);
				}


				return dxRenderObjData;
			}

			void DxRenderObjProxy::CreateTexture(H::Size size) {
				HRESULT hr = S_OK;

				auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
				auto d3dDev = dxDev->GetD3DDevice();

				Microsoft::WRL::ComPtr<ID3D11Texture2D> dxgiSwapChainBackBuffer;
				hr = this->swapChainPanel->GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&dxgiSwapChainBackBuffer));
				H::System::ThrowIfFailed(hr);

				// Retrieve texture description from swapChain but change some params.
				D3D11_TEXTURE2D_DESC texDesc = {};
				dxgiSwapChainBackBuffer->GetDesc(&texDesc);
				texDesc.Format = this->textureFormat;
				texDesc.Width = size.width;
				texDesc.Height = size.height;
				texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

				hr = d3dDev->CreateTexture2D(&texDesc, nullptr, this->dxRenderObjData->texture.ReleaseAndGetAddressOf());
				H::System::ThrowIfFailed(hr);

				CD3D11_RENDER_TARGET_VIEW_DESC descRTV(D3D11_RTV_DIMENSION_TEXTURE2D, texDesc.Format);
				hr = d3dDev->CreateRenderTargetView(this->dxRenderObjData->texture.Get(), &descRTV, this->dxRenderObjData->textureRTV.ReleaseAndGetAddressOf());
				H::System::ThrowIfFailed(hr);

				CD3D11_SHADER_RESOURCE_VIEW_DESC descSRV(D3D11_SRV_DIMENSION_TEXTURE2D, texDesc.Format, 0, 1);
				hr = d3dDev->CreateShaderResourceView(this->dxRenderObjData->texture.Get(), &descSRV, this->dxRenderObjData->textureSRV.ReleaseAndGetAddressOf());
				H::System::ThrowIfFailed(hr);
			}
		}
    }
}