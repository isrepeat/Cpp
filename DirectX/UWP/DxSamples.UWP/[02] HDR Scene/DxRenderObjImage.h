#pragma once
#include "DxRenderObj.h"

namespace DxSamples {
	class DxRenderObjImage : public DxRenderObjProvider<DxRenderObj> {
	public:
		DxRenderObjImage(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel)
			: swapChainPanel{ swapChainPanel }
		{
			this->dxRenderObj = this->CreateObject(this->swapChainPanel->GetDxDevice());
		}

		void CreateWindowSizeDependentResources() override {
			auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
			auto d3dDev = dxDev->GetD3DDevice();

			auto outputSize = this->swapChainPanel->GetOutputSize();
			float outputAR = outputSize.width / outputSize.height;
				
			D3D11_TEXTURE2D_DESC texDesc;
			this->dxRenderObj->texture->GetDesc(&texDesc);
			float texAR = float(texDesc.Width) / texDesc.Height;

			float scaleFactor = 1.5f;
			DirectX::XMStoreFloat4x4(
				&this->dxRenderObj->vsConstantBufferData.mWorldViewProj,
				DirectX::XMMatrixTranspose(
					DirectX::XMMatrixScaling(
						(scaleFactor / outputAR) * texAR,
						scaleFactor,
						scaleFactor
					)
				)
			);
		}

		void ReleaseDeviceDependentResources() override {
			this->dxRenderObj->Reset();
		}

	private:
		std::unique_ptr<DxRenderObj> CreateObject(H::Dx::DxDeviceSafeObj* dxDeviceSafeObj) {
			HRESULT hr = S_OK;

			auto dxDev = dxDeviceSafeObj->Lock();
			auto d3dDev = dxDev->GetD3DDevice();
			auto dxCtx = dxDev->LockContext();
			auto d2dCtx = dxCtx->D2D();

			auto dxRenderObj = std::make_unique<DxRenderObj>();

			// Load and create shaders.
			auto vertexShaderBlob = Tools::ReadFile(g_shaderLoadDir / L"imageVS.cso");
			hr = d3dDev->CreateVertexShader(
				vertexShaderBlob.data(),
				vertexShaderBlob.size(),
				nullptr,
				dxRenderObj->vertexShader.ReleaseAndGetAddressOf()
			);
			H::System::ThrowIfFailed(hr);

			auto pixelShaderBlob = Tools::ReadFile(g_shaderLoadDir / L"imagePS.cso");
			hr = d3dDev->CreatePixelShader(
				pixelShaderBlob.data(),
				pixelShaderBlob.size(),
				nullptr,
				dxRenderObj->pixelShader.ReleaseAndGetAddressOf()
			);
			H::System::ThrowIfFailed(hr);

			// Create input layout.
			static const D3D11_INPUT_ELEMENT_DESC inputElementDesc[2] = {
				{ "SV_Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA,  0 },
				{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA , 0 },
			};

			hr = d3dDev->CreateInputLayout(
				inputElementDesc,
				_countof(inputElementDesc),
				vertexShaderBlob.data(),
				vertexShaderBlob.size(),
				dxRenderObj->inputLayout.ReleaseAndGetAddressOf()
			);
			H::System::ThrowIfFailed(hr);


			// Create vertex buffer.
			{
				static const VertexPositionTexcoord vertexData[4] = {
					{ { -0.5f, -0.5f, 0.1f, 1.0f }, { 0.0f, 1.0f } }, // LB
					{ { -0.5f,  0.5f, 0.1f, 1.0f }, { 0.0f, 0.0f } }, // LT
					{ {  0.5f,  0.5f, 0.1f, 1.0f }, { 1.0f, 0.0f } }, // RT
					{ {  0.5f, -0.5f, 0.1f, 1.0f }, { 1.0f, 1.0f } }, // RB
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
					dxRenderObj->vertexBuffer.ReleaseAndGetAddressOf()
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
					dxRenderObj->indexBuffer.ReleaseAndGetAddressOf()
				);
				H::System::ThrowIfFailed(hr);
			}

			// Create sampler.
			{
				D3D11_SAMPLER_DESC samplerDesc = {};
				samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
				samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
				samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
				samplerDesc.MaxAnisotropy = 1;
				samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

				samplerDesc.MinLOD = 0;
				samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

				hr = d3dDev->CreateSamplerState(&samplerDesc, dxRenderObj->sampler.ReleaseAndGetAddressOf());
				H::System::ThrowIfFailed(hr);
			}

			// Create texture.
			{
				D3D11_TEXTURE2D_DESC txtDesc = {};
				txtDesc.MipLevels = txtDesc.ArraySize = 1;
				//txtDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
				txtDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
				txtDesc.SampleDesc.Count = 1;
				txtDesc.Usage = D3D11_USAGE_IMMUTABLE;
				txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

				auto imageBlob = Tools::LoadBGRAImage(L"Textures\\Dune_screenshot_24s.png", txtDesc.Width, txtDesc.Height);

				D3D11_SUBRESOURCE_DATA initialData = {};
				initialData.pSysMem = imageBlob.data();
				initialData.SysMemPitch = txtDesc.Width * sizeof(uint32_t);

				hr = d3dDev->CreateTexture2D(&txtDesc, &initialData, dxRenderObj->texture.GetAddressOf());
				H::System::ThrowIfFailed(hr);

				hr = d3dDev->CreateShaderResourceView(dxRenderObj->texture.Get(), nullptr, dxRenderObj->textureSRV.ReleaseAndGetAddressOf());
				H::System::ThrowIfFailed(hr);
			}

			// VS constant buffer.
			{
				DirectX::XMStoreFloat4x4(
					&dxRenderObj->vsConstantBufferData.mWorldViewProj,
					DirectX::XMMatrixTranspose(
						DirectX::XMMatrixIdentity()
					)
				);

				D3D11_BUFFER_DESC constantbufferDesc;
				constantbufferDesc.ByteWidth = sizeof(VS_CONSTANT_BUFFER);
				constantbufferDesc.Usage = D3D11_USAGE_DEFAULT;
				constantbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				constantbufferDesc.CPUAccessFlags = 0;
				constantbufferDesc.MiscFlags = 0;
				constantbufferDesc.StructureByteStride = 0;

				hr = d3dDev->CreateBuffer(
					&constantbufferDesc,
					nullptr,
					dxRenderObj->vsConstantBuffer.ReleaseAndGetAddressOf()
				);
				H::System::ThrowIfFailed(hr);
			}

			// PS constant buffer.
			{
				// Store default values
				dxRenderObj->psConstantBufferData.currentNits = { 0.0f /*this->currentNits*/, 0.0f, 0.0f, 0.0f };

				D3D11_BUFFER_DESC constantbufferDesc;
				constantbufferDesc.ByteWidth = sizeof(PS_CONSTANT_BUFFER);
				constantbufferDesc.Usage = D3D11_USAGE_DEFAULT;
				constantbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				constantbufferDesc.CPUAccessFlags = 0;
				constantbufferDesc.MiscFlags = 0;
				constantbufferDesc.StructureByteStride = 0;

				hr = d3dDev->CreateBuffer(
					&constantbufferDesc,
					nullptr,
					dxRenderObj->psConstantBuffer.ReleaseAndGetAddressOf()
				);
				H::System::ThrowIfFailed(hr);
			}

			return dxRenderObj;
		}


	private:
		Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel;
	};
}