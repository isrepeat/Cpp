#include "SimpleSceneRenderer.h"
#include <Helpers/Dx/DxHelpers.h>
#include <Helpers/Helpers.h>

#include "Tools.h"
#include <DirectXMath.h>
#include <fstream>

#pragma comment(lib, "dxguid.lib")

namespace {
	struct VertexPositionTexcoord {
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT2 texcoord;
	};
	struct VertexPositionColor {
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 color;
	};
}

namespace DxDesktop {
	SimpleSceneRenderer::SimpleSceneRenderer(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel)
		: swapChainPanel{ swapChainPanel }
	{
		HRESULT hr = S_OK;

		this->CreateDeviceDependentResources();
		this->CreateWindowSizeDependentResources();
	}

	SimpleSceneRenderer::~SimpleSceneRenderer()
	{}

	// Executes basic render loop.
	void SimpleSceneRenderer::Tick() {
		this->Render();
	}


	void SimpleSceneRenderer::CreateDeviceDependentResources() {
		HRESULT hr = S_OK;

		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDev = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d2dCtx = dxCtx->D2D();

		this->dxRenderObjImage = this->CreateDxRenderObjImage();
		this->dxRenderObjTriangle = this->CreateDxRenderObjTriangle();
	}

	void SimpleSceneRenderer::CreateWindowSizeDependentResources() {
	}

	void SimpleSceneRenderer::ReleaseDeviceDependentResources() {
		this->dxRenderObjImage.Reset();
		this->dxRenderObjTriangle.Reset();
	}


	DxRenderObj SimpleSceneRenderer::CreateDxRenderObjImage() {
		HRESULT hr = S_OK;
		DxRenderObj dxRenderObj;

		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDev = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d2dCtx = dxCtx->D2D();

		// Load and create shaders.
		auto vertexShaderBlob = Tools::ReadFile(H::ExePath() / L"imageVS.cso");

		hr = d3dDev->CreateVertexShader(
			vertexShaderBlob.data(),
			vertexShaderBlob.size(),
			nullptr,
			dxRenderObj.vertexShader.ReleaseAndGetAddressOf()
		);
		H::System::ThrowIfFailed(hr);


		auto pixelShaderBlob = Tools::ReadFile(H::ExePath() / L"imagePS.cso");

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

		// Create linear sampler.
		{
			D3D11_SAMPLER_DESC samplerDesc = {};
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

			//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			//samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			//samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			//samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			//samplerDesc.MaxAnisotropy = 1;
			//samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

			samplerDesc.MinLOD = 0;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			hr = d3dDev->CreateSamplerState(&samplerDesc, dxRenderObj.sampler.ReleaseAndGetAddressOf());
			H::System::ThrowIfFailed(hr);
		}

		// Create texture.
		{
			D3D11_TEXTURE2D_DESC txtDesc = {};
			txtDesc.MipLevels = txtDesc.ArraySize = 1;
			txtDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // sunset.jpg is in sRGB colorspace
			txtDesc.SampleDesc.Count = 1;
			txtDesc.Usage = D3D11_USAGE_IMMUTABLE;
			txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			//auto imageBlob = Tools::LoadBGRAImage(L"Textures\\textureBrick.bmp", txtDesc.Width, txtDesc.Height);
			//auto imageBlob = Tools::LoadBGRAImage(L"Textures\\rosendal_plains_2.webp", txtDesc.Width, txtDesc.Height);
			auto imageBlob = Tools::LoadBGRAImage(L"Textures\\Dune_screenshot_24s.png", txtDesc.Width, txtDesc.Height);

			D3D11_SUBRESOURCE_DATA initialData = {};
			initialData.pSysMem = imageBlob.data();
			initialData.SysMemPitch = txtDesc.Width * sizeof(uint32_t);

			Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
			hr = d3dDev->CreateTexture2D(&txtDesc, &initialData, tex.GetAddressOf());
			H::System::ThrowIfFailed(hr);

			hr = d3dDev->CreateShaderResourceView(tex.Get(), nullptr, dxRenderObj.texture.ReleaseAndGetAddressOf());
			H::System::ThrowIfFailed(hr);
		}

		// Constant buffer.
		{
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

			//D3D11_SUBRESOURCE_DATA initData;
			//initData.pSysMem = &dxRenderObj.vsConstantBufferData;
			//initData.SysMemPitch = 0;
			//initData.SysMemSlicePitch = 0;

			hr = d3dDev->CreateBuffer(
				&constantbufferDesc,
				nullptr, // &initData,
				dxRenderObj.vsConstantBuffer.ReleaseAndGetAddressOf()
			);
			H::System::ThrowIfFailed(hr);
		}

		return dxRenderObj;
	}

	DxRenderObj SimpleSceneRenderer::CreateDxRenderObjTriangle() {
		HRESULT hr = S_OK;
		DxRenderObj dxRenderObj;

		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDev = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d2dCtx = dxCtx->D2D();

		// Load and create shaders.
		auto vertexShaderBlob = Tools::ReadFile(H::ExePath() / L"triangleVS.cso");

		hr = d3dDev->CreateVertexShader(
			vertexShaderBlob.data(),
			vertexShaderBlob.size(),
			nullptr,
			dxRenderObj.vertexShader.ReleaseAndGetAddressOf()
		);
		H::System::ThrowIfFailed(hr);


		auto pixelShaderBlob = Tools::ReadFile(H::ExePath() / L"trianglePS.cso");

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
			{ "COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA , 0 },
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
			static const VertexPositionColor vertexData[3] = {
				{ { -0.5f,  0.5f,  0.1f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }, // Left-top / Red
				{ {  0.5f,  0.5f,  0.1f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // Right-top / Green
				{ {  0.5f, -0.5f,  0.1f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }, // Right-bottom / Blue
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
		}

		// Constant buffer.
		{
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

			//D3D11_SUBRESOURCE_DATA initData;
			//initData.pSysMem = &dxRenderObj.vsConstantBufferData;
			//initData.SysMemPitch = 0;
			//initData.SysMemSlicePitch = 0;

			hr = d3dDev->CreateBuffer(
				&constantbufferDesc,
				nullptr, // &initData,
				dxRenderObj.vsConstantBuffer.ReleaseAndGetAddressOf()
			);
			H::System::ThrowIfFailed(hr);
		}

		return dxRenderObj;
	}

	// Helper method to clear the back buffers.
	void SimpleSceneRenderer::Clear() {
		HRESULT hr = S_OK;

		// Clear the views.
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		auto renderTargetView1 = this->swapChainPanel->GetRenderTargetView();

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
		hr = renderTargetView1.As(&renderTargetView);
		H::System::ThrowIfFailed(hr);

		// Use linear clear color for gamma-correct rendering.
		d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Black);

		auto pRenderTargetView = renderTargetView.Get();
		d3dCtx->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

		// Set the viewport.
		auto viewport = this->swapChainPanel->GetScreenViewport();
		d3dCtx->RSSetViewports(1, &viewport);
	}

	void SimpleSceneRenderer::Render() {
		this->Clear();

		this->RenderImage();
		this->RenderTriangle();
	}

	void SimpleSceneRenderer::RenderImage() {
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDevice = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		auto& dxRenderObj = this->dxRenderObjImage;

		// Update contant buffer data
		DirectX::XMStoreFloat4x4(
			&dxRenderObj.vsConstantBufferData.mWorldViewProj,
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixScaling(0.8f, 0.8f, 0.8f) * DirectX::XMMatrixTranslation(-0.5f, 0.0f, 0.0f)
			)
		);

		d3dCtx->UpdateSubresource(
			dxRenderObj.vsConstantBuffer.Get(),
			0,
			nullptr,
			&dxRenderObj.vsConstantBufferData,
			0,
			0
		);

		// Set input assembler state.
		d3dCtx->IASetInputLayout(dxRenderObj.inputLayout.Get());

		UINT strides = sizeof(VertexPositionTexcoord);
		UINT offsets = 0;
		d3dCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		d3dCtx->IASetVertexBuffers(0, 1, dxRenderObj.vertexBuffer.GetAddressOf(), &strides, &offsets);

		d3dCtx->IASetIndexBuffer(dxRenderObj.indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		// Set shaders.
		d3dCtx->VSSetShader(dxRenderObj.vertexShader.Get(), nullptr, 0);
		d3dCtx->VSSetConstantBuffers(0, 1, dxRenderObj.vsConstantBuffer.GetAddressOf());
		d3dCtx->PSSetShader(dxRenderObj.pixelShader.Get(), nullptr, 0);

		// Set texture and sampler.
		auto pTexture = dxRenderObj.texture.Get();
		d3dCtx->PSSetShaderResources(0, 1, &pTexture);

		auto pSampler = dxRenderObj.sampler.Get();
		d3dCtx->PSSetSamplers(0, 1, &pSampler);

		// Draw quad.
		d3dCtx->DrawIndexed(6, 0, 0);
	}

	void SimpleSceneRenderer::RenderTriangle() {
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		auto& dxRenderObj = this->dxRenderObjTriangle;

		// Update contant buffer data
		DirectX::XMStoreFloat4x4(
			&dxRenderObj.vsConstantBufferData.mWorldViewProj,
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixScaling(0.8f, 0.8f, 0.8f) * DirectX::XMMatrixTranslation(0.5f, 0.0f, 0.0f)
			)
		);

		d3dCtx->UpdateSubresource(
			dxRenderObj.vsConstantBuffer.Get(),
			0,
			nullptr,
			&dxRenderObj.vsConstantBufferData,
			0,
			0
		);

		// Set input assembler state.
		d3dCtx->IASetInputLayout(dxRenderObj.inputLayout.Get());

		UINT strides = sizeof(VertexPositionColor);
		UINT offsets = 0;
		d3dCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		d3dCtx->IASetVertexBuffers(0, 1, dxRenderObj.vertexBuffer.GetAddressOf(), &strides, &offsets);

		d3dCtx->IASetIndexBuffer(dxRenderObj.indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		// Set shaders.
		d3dCtx->VSSetShader(dxRenderObj.vertexShader.Get(), nullptr, 0);
		d3dCtx->VSSetConstantBuffers(0, 1, dxRenderObj.vsConstantBuffer.GetAddressOf());
		d3dCtx->PSSetShader(dxRenderObj.pixelShader.Get(), nullptr, 0);

		// Draw quad.
		d3dCtx->Draw(3, 0);
	}
}