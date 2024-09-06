#include "QuadRenderer.h"
#include "Tools.h"
#include <exception>

namespace DxSamples {
	QuadRenderer::QuadRenderer(H::Dx::DxDeviceSafeObj* dxDeviceSafeObj)
		: dxDeviceSafeObj{ dxDeviceSafeObj }
	{
		HRESULT hr = S_OK;

		auto dxDev = this->dxDeviceSafeObj->Lock();
		auto d3dDev = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d2dCtx = dxCtx->D2D();

		auto& dxRenderObj = this->defaultRenderObj;

		// Load and create shaders.
		auto vertexShaderBlob = Tools::ReadFile(g_shaderLoadDir / L"Default_VS.cso");

		hr = d3dDev->CreateVertexShader(
			vertexShaderBlob.data(),
			vertexShaderBlob.size(),
			nullptr,
			dxRenderObj.vertexShader.ReleaseAndGetAddressOf()
		);
		H::System::ThrowIfFailed(hr);


		auto pixelShaderBlob = Tools::ReadFile(g_shaderLoadDir / L"Default_PS.cso");

		hr = d3dDev->CreatePixelShader(
			pixelShaderBlob.data(),
			pixelShaderBlob.size(),
			nullptr,
			dxRenderObj.pixelShader.ReleaseAndGetAddressOf()
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
			dxRenderObj.inputLayout.ReleaseAndGetAddressOf()
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
				dxRenderObj.vertexBuffer.ReleaseAndGetAddressOf()
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
				dxRenderObj.indexBuffer.ReleaseAndGetAddressOf()
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

			hr = d3dDev->CreateSamplerState(&samplerDesc, dxRenderObj.sampler.ReleaseAndGetAddressOf());
			H::System::ThrowIfFailed(hr);
		}

		// VS constant buffer.
		{
			float scaleFactor = 1.0f;
			DirectX::XMStoreFloat4x4(
				&dxRenderObj.vsConstantBufferData.mWorldViewProj,
				DirectX::XMMatrixTranspose(
					DirectX::XMMatrixIdentity()
				)
			);

			D3D11_BUFFER_DESC constantbufferDesc;
			constantbufferDesc.ByteWidth = sizeof(VS_CONSTANT_BUFFER);
			constantbufferDesc.Usage = D3D11_USAGE_DEFAULT; // D3D11_USAGE_DYNAMIC;
			constantbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			constantbufferDesc.CPUAccessFlags = 0; // D3D11_CPU_ACCESS_WRITE;
			constantbufferDesc.MiscFlags = 0;
			constantbufferDesc.StructureByteStride = 0;

			hr = d3dDev->CreateBuffer(
				&constantbufferDesc,
				nullptr,
				dxRenderObj.vsConstantBuffer.ReleaseAndGetAddressOf()
			);
			H::System::ThrowIfFailed(hr);
		}

		// PS constant buffer.
		{
			// Store default values
			dxRenderObj.psConstantBufferData.currentNits = { 0.0f, 0.0f, 0.0f, 0.0f };

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
				dxRenderObj.psConstantBuffer.ReleaseAndGetAddressOf()
			);
			H::System::ThrowIfFailed(hr);
		}
	}



	void QuadRenderer::Draw(
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV,
		std::function<void __cdecl()> setCustomState)
	{
		auto dxDev = this->dxDeviceSafeObj->Lock();
		auto d3dDevice = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		DxRenderObj* dxRenderObj = &this->defaultRenderObj;

		// Update constant buffer data
		d3dCtx->UpdateSubresource(dxRenderObj->vsConstantBuffer.Get(), 0, nullptr, &dxRenderObj->vsConstantBufferData, 0, 0);

		// Set texture and sampler.
		auto pTextureSRV = textureSRV.Get();
		d3dCtx->PSSetShaderResources(0, 1, &pTextureSRV);

		auto pSampler = dxRenderObj->sampler.Get();
		d3dCtx->PSSetSamplers(0, 1, &pSampler);

		// Set shaders.
		d3dCtx->VSSetShader(dxRenderObj->vertexShader.Get(), nullptr, 0);
		d3dCtx->PSSetShader(dxRenderObj->pixelShader.Get(), nullptr, 0);
		d3dCtx->VSSetConstantBuffers(0, 1, dxRenderObj->vsConstantBuffer.GetAddressOf());
		d3dCtx->PSSetConstantBuffers(0, 1, dxRenderObj->psConstantBuffer.GetAddressOf());

		if (setCustomState) {
			setCustomState();
		}

		// Set input layout, vertex / index buffers.
		d3dCtx->IASetInputLayout(dxRenderObj->inputLayout.Get());
		d3dCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT strides = sizeof(VertexPositionTexcoord);
		UINT offsets = 0;
		d3dCtx->IASetVertexBuffers(0, 1, dxRenderObj->vertexBuffer.GetAddressOf(), &strides, &offsets);
		d3dCtx->IASetIndexBuffer(dxRenderObj->indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		// Draw quad.
		d3dCtx->DrawIndexed(6, 0, 0);
	}


	void QuadRenderer::Draw(
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV,
		DxRenderObjBase* outterRenderObj,
		std::function<void __cdecl()> setCustomState)
	{
		auto dxDev = this->dxDeviceSafeObj->Lock();
		auto d3dDevice = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		DxRenderObjBase* dxRenderObjBase = outterRenderObj;

		// Update constant buffer data
		d3dCtx->UpdateSubresource(dxRenderObjBase->vsConstantBuffer.Get(), 0, nullptr, &dxRenderObjBase->vsConstantBufferData, 0, 0);

		// Set texture and sampler.
		auto pTextureSRV = textureSRV.Get();
		d3dCtx->PSSetShaderResources(0, 1, &pTextureSRV);

		auto pSampler = dxRenderObjBase->sampler.Get();
		d3dCtx->PSSetSamplers(0, 1, &pSampler);

		if (setCustomState) {
			setCustomState(); // Set shaders, buffers ...
		}

		// Set input layout, vertex / index buffers.
		d3dCtx->IASetInputLayout(dxRenderObjBase->inputLayout.Get());
		d3dCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT strides = sizeof(VertexPositionTexcoord);
		UINT offsets = 0;
		d3dCtx->IASetVertexBuffers(0, 1, dxRenderObjBase->vertexBuffer.GetAddressOf(), &strides, &offsets);
		d3dCtx->IASetIndexBuffer(dxRenderObjBase->indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		// Draw quad.
		d3dCtx->DrawIndexed(6, 0, 0);
	}
}