#include "DxRenderObjHDR.h"
#include <Helpers/Dx/Shaders/ShadersCommon.h>
#include <Helpers/FileSystem.h>

namespace HELPERS_NS {
    namespace Dx {
		namespace details {
			DxRenderObjHDR::DxRenderObjHDR(
				Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel,
				Params params
			)
				: swapChainPanel{ swapChainPanel }
			{
				LOG_ASSERT(swapChainPanel);

				this->dxRenderObjData = this->CreateObjectData(
					this->swapChainPanel->GetDxDevice(),
					params
				);
				this->CreateWindowSizeDependentResources();
			}

			DxRenderObjHDR::DxRenderObjHDR(
				DxDeviceSafeObj* dxDeviceSafeObj,
				Params params)
				: DxRenderObjHDR(dxDeviceSafeObj->Lock()->GetAssociatedSwapChainPanel(), params)
			{
			}


			void DxRenderObjHDR::CreateWindowSizeDependentResources(std::optional<H::Size> size) {
				H::Size newSize;
				if (size) {
					newSize = size.value();
				}
				else {
					newSize = static_cast<H::Size>(this->swapChainPanel->GetOutputSize());
				}
				this->CreateTexture(newSize);

				this->events.additionalSizeDependentHandlers.Invoke(newSize);
			}

			void DxRenderObjHDR::ReleaseDeviceDependentResources() {
				this->dxRenderObjData->Reset();
			}

			void DxRenderObjHDR::UpdateBuffers() {
				auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
				auto d3dDev = dxDev->GetD3DDevice();
				auto dxCtx = dxDev->LockContext();
				auto d3dCtx = dxCtx->D3D();

				d3dCtx->UpdateSubresource(this->dxRenderObjData->vsConstantBuffer.Get(), 0, nullptr, &this->dxRenderObjData->vsConstantBufferData, 0, 0);
				d3dCtx->UpdateSubresource(this->dxRenderObjData->psConstantBuffer.Get(), 0, nullptr, &this->dxRenderObjData->psConstantBufferData, 0, 0);
			}


            const DxRenderObjHDR::Events& DxRenderObjHDR::GetEvents() const {
				return this->events;
            }

            DxTextureResources DxRenderObjHDR::GetTextureResources() {
				auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
				DxTextureResources dxTextureResources;
				dxTextureResources.texture = this->dxRenderObjData->texture;
				dxTextureResources.textureRTV = this->dxRenderObjData->textureRTV;
				dxTextureResources.textureSRV = this->dxRenderObjData->textureSRV;
				dxTextureResources.bitmap = this->dxRenderObjData->bitmap;
				return dxTextureResources;
			}

			std::unique_ptr<DxRenderObjHDRData> DxRenderObjHDR::CreateObjectData(
				H::Dx::DxDeviceSafeObj* dxDeviceSafeObj,
				Params params)
			{
				HRESULT hr = S_OK;

				auto dxDev = dxDeviceSafeObj->Lock();
				auto d3dDev = dxDev->GetD3DDevice();
				auto dxCtx = dxDev->LockContext();
				auto d2dCtx = dxCtx->D2D();

				auto dxRenderObjData = std::make_unique<DxRenderObjHDRData>();
				
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
					if (params.vertexShader.empty()) {
						params.vertexShader = g_shaderLoadDir / L"defaultVS.cso";
					}
					else {
						params.vertexShader = H::ExePath() / params.vertexShader;
					}

					auto vertexShaderBlob = H::FS::ReadFile(params.vertexShader);
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
					if (params.pixelShaderHDR.empty()) {
						params.pixelShaderHDR = g_shaderLoadDir / L"defaultPS.cso";
					}
					else {
						params.pixelShaderHDR = H::ExePath() / params.pixelShaderHDR;
					}

					if (params.pixelShaderToneMap.empty()) {
						params.pixelShaderToneMap = g_shaderLoadDir / L"defaultPS.cso";
					}
					else {
						params.pixelShaderToneMap = H::ExePath() / params.pixelShaderToneMap;
					}

					auto pixelShaderHDRBlob = H::FS::ReadFile(params.pixelShaderHDR);
					hr = d3dDev->CreatePixelShader(
						pixelShaderHDRBlob.data(),
						pixelShaderHDRBlob.size(),
						nullptr,
						dxRenderObjData->pixelShaderHDR.ReleaseAndGetAddressOf()
					);
					H::System::ThrowIfFailed(hr);

					auto pixelShaderToneMapBlob = H::FS::ReadFile(params.pixelShaderToneMap);
					hr = d3dDev->CreatePixelShader(
						pixelShaderToneMapBlob.data(),
						pixelShaderToneMapBlob.size(),
						nullptr,
						dxRenderObjData->pixelShaderToneMap.ReleaseAndGetAddressOf()
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
					constantbufferDesc.ByteWidth = sizeof(DxRenderObjHDRData::VS_CONSTANT_BUFFER);
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
					dxRenderObjData->psConstantBufferData.luminance = { 0.0f, 0.0f, 0.0f, 0.0f };

					D3D11_BUFFER_DESC constantbufferDesc;
					constantbufferDesc.ByteWidth = sizeof(DxRenderObjHDRData::PS_CONSTANT_BUFFER);
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

			void DxRenderObjHDR::CreateTexture(H::Size size) {
				HRESULT hr = S_OK;

				auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
				auto dxCtx = dxDev->LockContext();
				auto d3dDev = dxDev->GetD3DDevice();
				auto d2dCtx = dxCtx->D2D();

				CD3D11_TEXTURE2D_DESC texDesc(DXGI_FORMAT_R16G16B16A16_FLOAT, size.width, size.height, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
				hr = d3dDev->CreateTexture2D(&texDesc, nullptr, this->dxRenderObjData->texture.ReleaseAndGetAddressOf());
				H::System::ThrowIfFailed(hr);

				CD3D11_RENDER_TARGET_VIEW_DESC descRTV(D3D11_RTV_DIMENSION_TEXTURE2D, texDesc.Format);
				hr = d3dDev->CreateRenderTargetView(this->dxRenderObjData->texture.Get(), &descRTV, this->dxRenderObjData->textureRTV.ReleaseAndGetAddressOf());
				H::System::ThrowIfFailed(hr);

				CD3D11_SHADER_RESOURCE_VIEW_DESC descSRV(D3D11_SRV_DIMENSION_TEXTURE2D, texDesc.Format, 0, 1);
				hr = d3dDev->CreateShaderResourceView(this->dxRenderObjData->texture.Get(), &descSRV, this->dxRenderObjData->textureSRV.ReleaseAndGetAddressOf());
				H::System::ThrowIfFailed(hr);

				Microsoft::WRL::ComPtr<IDXGISurface2> dxgiSurface;
				hr = this->dxRenderObjData->texture.As(&dxgiSurface);
				H::System::ThrowIfFailed(hr);

				D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_TARGET,
					D2D1::PixelFormat(texDesc.Format, D2D1_ALPHA_MODE_PREMULTIPLIED),
					this->swapChainPanel->GetDpi(),
					this->swapChainPanel->GetDpi()
				);

				hr = d2dCtx->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &bitmapProperties, this->dxRenderObjData->bitmap.ReleaseAndGetAddressOf());
				H::System::ThrowIfFailed(hr);
			}
		}
    }
}