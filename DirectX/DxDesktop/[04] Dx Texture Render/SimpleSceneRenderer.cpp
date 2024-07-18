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
		, keyboard{ std::make_unique<DirectX::Keyboard>() }
		, currentNits{ 1500.0f }
	{
		HRESULT hr = S_OK;

		this->CreateDeviceDependentResources();
		this->CreateWindowSizeDependentResources();
	}

	SimpleSceneRenderer::~SimpleSceneRenderer()
	{}

	// Executes basic render loop.
	void SimpleSceneRenderer::Tick() {
		this->renderLoopTimer.Tick([this] {
			this->Update();
		});

		this->Render();
	}

	void SimpleSceneRenderer::OnWindowSizeChanged(H::Size newSize) {
		this->swapChainPanel->SetLogicalSize(static_cast<H::Size_f>(newSize));
		this->CreateWindowSizeDependentResources();
	}


	void SimpleSceneRenderer::CreateDeviceDependentResources() {
		HRESULT hr = S_OK;

		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDev = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d2dCtx = dxCtx->D2D();

		this->dxRenderObjImage = this->CreateDxRenderObjImage();
		this->dxRenderObjHdrQuad = this->CreateDxRenderObjHdrQuad();
	}

	void SimpleSceneRenderer::CreateWindowSizeDependentResources() {
		{
			auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
			auto d3dDev = dxDev->GetD3DDevice();

			auto outputSize = this->swapChainPanel->GetOutputSize();
			float outputAR = outputSize.width / outputSize.height;
			
			D3D11_TEXTURE2D_DESC texDesc;
			this->dxRenderObjImage.texture->GetDesc(&texDesc);
			float texAR = float(texDesc.Width) / texDesc.Height;

			float scaleFactor = 1.5f;
			DirectX::XMStoreFloat4x4(
				&this->dxRenderObjImage.vsConstantBufferData.mWorldViewProj,
				DirectX::XMMatrixTranspose(
					DirectX::XMMatrixScaling(
						(scaleFactor / outputAR) * texAR,
						scaleFactor,
						scaleFactor
					)
				)
			);
		}

		this->CreateHdrTexture(&this->dxRenderObjHdrQuad);
	}

	void SimpleSceneRenderer::ReleaseDeviceDependentResources() {
		this->dxRenderObjImage.Reset();
		this->dxRenderObjHdrQuad.Reset();
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

		// Create texture.
		{
			D3D11_TEXTURE2D_DESC txtDesc = {};
			txtDesc.MipLevels = txtDesc.ArraySize = 1;
			txtDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // sunset.jpg is in sRGB colorspace
			txtDesc.SampleDesc.Count = 1;
			txtDesc.Usage = D3D11_USAGE_IMMUTABLE;
			txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			//auto imageBlob = Tools::LoadBGRAImage(L"Textures\\rosendal_plains.webp", txtDesc.Width, txtDesc.Height);
			auto imageBlob = Tools::LoadBGRAImage(L"Textures\\Dune_screenshot_24s.png", txtDesc.Width, txtDesc.Height);

			D3D11_SUBRESOURCE_DATA initialData = {};
			initialData.pSysMem = imageBlob.data();
			initialData.SysMemPitch = txtDesc.Width * sizeof(uint32_t);

			hr = d3dDev->CreateTexture2D(&txtDesc, &initialData, dxRenderObj.texture.GetAddressOf());
			H::System::ThrowIfFailed(hr);

			hr = d3dDev->CreateShaderResourceView(dxRenderObj.texture.Get(), nullptr, dxRenderObj.textureSRV.ReleaseAndGetAddressOf());
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
			dxRenderObj.psConstantBufferData.currentNits = { this->currentNits, 0.0f, 0.0f, 0.0f };

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

		return dxRenderObj;
	}

	DxRenderObjHDR SimpleSceneRenderer::CreateDxRenderObjHdrQuad() {
		HRESULT hr = S_OK;
		DxRenderObjHDR dxRenderObj;

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

		auto pixelShaderHDRBlob = Tools::ReadFile(H::ExePath() / L"PrepareSwapChainBuffersPS.cso");
		hr = d3dDev->CreatePixelShader(
			pixelShaderHDRBlob.data(),
			pixelShaderHDRBlob.size(),
			nullptr,
			dxRenderObj.pixelShaderHDR.ReleaseAndGetAddressOf()
		);
		H::System::ThrowIfFailed(hr);

		auto pixelShaderToneMapBlob = Tools::ReadFile(H::ExePath() / L"ToneMapPS.cso");
		hr = d3dDev->CreatePixelShader(
			pixelShaderToneMapBlob.data(),
			pixelShaderToneMapBlob.size(),
			nullptr,
			dxRenderObj.pixelShaderToneMap.ReleaseAndGetAddressOf()
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
			float w = 0.9f;
			float h = 0.9f;
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

		// Create texture.
		{
			this->CreateHdrTexture(&dxRenderObj);
		}

		// VS constant buffer.
		{
			DirectX::XMStoreFloat4x4(
				&dxRenderObj.vsConstantBufferData.mWorldViewProj,
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
				dxRenderObj.vsConstantBuffer.ReleaseAndGetAddressOf()
			);
			H::System::ThrowIfFailed(hr);
		}

		// PS constant buffer.
		{
			// Store default values
			dxRenderObj.psConstantBufferData.currentNits = { this->currentNits, 0.0f, 0.0f, 0.0f };

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

		return dxRenderObj;
	}

	void SimpleSceneRenderer::CreateHdrTexture(DxRenderObjBase* dxRenderObjBase) {
		HRESULT hr = S_OK;

		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDev = dxDev->GetD3DDevice();

		auto outputSize = this->swapChainPanel->GetOutputSize();
		int width = outputSize.width;
		int height = outputSize.height;

		CD3D11_TEXTURE2D_DESC descTex(DXGI_FORMAT_R16G16B16A16_FLOAT, width, height, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
		hr = d3dDev->CreateTexture2D(&descTex, nullptr, dxRenderObjBase->texture.ReleaseAndGetAddressOf());
		H::System::ThrowIfFailed(hr);

		CD3D11_RENDER_TARGET_VIEW_DESC descRTV(D3D11_RTV_DIMENSION_TEXTURE2D, descTex.Format);
		hr = d3dDev->CreateRenderTargetView(dxRenderObjBase->texture.Get(), &descRTV, dxRenderObjBase->textureRTV.ReleaseAndGetAddressOf());
		H::System::ThrowIfFailed(hr);

		CD3D11_SHADER_RESOURCE_VIEW_DESC descSRV(D3D11_SRV_DIMENSION_TEXTURE2D, descTex.Format, 0, 1);
		hr = d3dDev->CreateShaderResourceView(dxRenderObjBase->texture.Get(), &descSRV, dxRenderObjBase->textureSRV.ReleaseAndGetAddressOf());
		H::System::ThrowIfFailed(hr);
	}



	void SimpleSceneRenderer::Update() {
		using DirectX::Keyboard;

		auto kb = this->keyboard->GetState();
		this->keyboardButtons.Update(kb);

		bool nitsChanged = false;

		if (kb.Escape) {
			//ExitSample();
		}

		if (this->keyboardButtons.IsKeyPressed(Keyboard::Space)) {
		}

		if (this->keyboardButtons.IsKeyPressed(Keyboard::Enter)) {
		}

		if (this->keyboardButtons.IsKeyPressed(Keyboard::OemMinus) || this->keyboardButtons.IsKeyPressed(Keyboard::Subtract)) {
			nitsChanged = true;
			this->currentNits -= 200.0f;
			this->currentNits = std::max(this->currentNits, 200.0f);
		}

		if (this->keyboardButtons.IsKeyPressed(Keyboard::OemPlus) || this->keyboardButtons.IsKeyPressed(Keyboard::Add)) {
			nitsChanged = true;
			this->currentNits += 200.0f;
			this->currentNits = std::max(this->currentNits, DX::c_MaxNitsFor2084);
		}

		if (kb.Down || kb.Left) {
		}

		if (kb.Up || kb.Right){
		}

		// Update constant buffer
		if (nitsChanged) {
			auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
			auto dxCtx = dxDev->LockContext();
			auto d3dCtx = dxCtx->D3D();

			for (auto* dxRenderObj : { &this->dxRenderObjImage }) {
				dxRenderObj->psConstantBufferData.currentNits = { this->currentNits, 0.0f, 0.0f, 0.0f };
				d3dCtx->UpdateSubresource(dxRenderObj->psConstantBuffer.Get(), 0, nullptr, &dxRenderObj->psConstantBufferData, 0, 0);
			}
		}
	}

	// Helper method to clear the back buffers.
	void SimpleSceneRenderer::Clear() {
		HRESULT hr = S_OK;

		// Clear the views.
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		//// Render to swapChain RTV.
		//auto renderTargetView1 = this->swapChainPanel->GetRenderTargetView();

		//Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
		//hr = renderTargetView1.As(&renderTargetView);
		//H::System::ThrowIfFailed(hr);

		//d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Gray);

		//ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
		//d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

		// Render to HDR quad RTV.
		auto renderTargetView = this->dxRenderObjHdrQuad.textureRTV;
		d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Black);

		ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
		d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

		// Set the viewport.
		auto viewport = this->swapChainPanel->GetScreenViewport();
		d3dCtx->RSSetViewports(1, &viewport);
	}

	void SimpleSceneRenderer::Render() {
		// Don't try to render anything before the first Update.
		if (this->renderLoopTimer.GetFrameCount() == 0) {
			return;
		}

		this->Clear();

		// Render the HDR scene with values larger than 1.0f, which will be perceived as bright
		this->RenderHDRScene();

		// Process the HDR scene so that the swapchains can correctly be sent to HDR or SDR display
		this->PrepareSwapChainBuffer();
	}

	void SimpleSceneRenderer::RenderHDRScene() {
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDevice = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		auto& dxRenderObj = this->dxRenderObjImage;

		// Update contant buffer data
		d3dCtx->UpdateSubresource(dxRenderObj.vsConstantBuffer.Get(), 0, nullptr, &dxRenderObj.vsConstantBufferData, 0, 0);

		// Set texture and sampler.
		auto pTextureSRV = dxRenderObj.textureSRV.Get();
		d3dCtx->PSSetShaderResources(0, 1, &pTextureSRV);

		auto pSampler = dxRenderObj.sampler.Get();
		d3dCtx->PSSetSamplers(0, 1, &pSampler);

		// Set shaders.
		d3dCtx->VSSetShader(dxRenderObj.vertexShader.Get(), nullptr, 0);
		d3dCtx->PSSetShader(dxRenderObj.pixelShader.Get(), nullptr, 0);
		d3dCtx->VSSetConstantBuffers(0, 1, dxRenderObj.vsConstantBuffer.GetAddressOf());
		d3dCtx->PSSetConstantBuffers(0, 1, dxRenderObj.psConstantBuffer.GetAddressOf());

		// Set input layout, vertex / index buffers.
		d3dCtx->IASetInputLayout(dxRenderObj.inputLayout.Get());
		d3dCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT strides = sizeof(VertexPositionTexcoord);
		UINT offsets = 0;
		d3dCtx->IASetVertexBuffers(0, 1, dxRenderObj.vertexBuffer.GetAddressOf(), &strides, &offsets);
		d3dCtx->IASetIndexBuffer(dxRenderObj.indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		// Draw quad.
		d3dCtx->DrawIndexed(6, 0, 0);
	}
	

	void SimpleSceneRenderer::PrepareSwapChainBuffer() {
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		HRESULT hr = S_OK;

		// Render to swapChain RTV.
		auto renderTargetView1 = this->swapChainPanel->GetRenderTargetView();

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
		hr = renderTargetView1.As(&renderTargetView);
		H::System::ThrowIfFailed(hr);

		d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Gray);

		ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
		d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

		auto viewport = this->swapChainPanel->GetScreenViewport();
		d3dCtx->RSSetViewports(1, &viewport);


		// Render a fullscreen quad and apply the HDR/SDR shaders.
		auto& dxRenderObj = this->dxRenderObjHdrQuad;

		// Update contant buffer data
		d3dCtx->UpdateSubresource(dxRenderObj.vsConstantBuffer.Get(), 0, nullptr, &dxRenderObj.vsConstantBufferData, 0, 0);

		// Set texture and sampler.
		auto pTextureSRV = dxRenderObj.textureSRV.Get();
		d3dCtx->PSSetShaderResources(0, 1, &pTextureSRV);

		auto pSampler = dxRenderObj.sampler.Get();
		d3dCtx->PSSetSamplers(0, 1, &pSampler);

		// Set shaders.
		d3dCtx->VSSetShader(dxRenderObj.vertexShader.Get(), nullptr, 0);
		d3dCtx->VSSetConstantBuffers(0, 1, dxRenderObj.vsConstantBuffer.GetAddressOf());
		
		if (this->swapChainPanel->GetColorSpace() == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020) {

			d3dCtx->PSSetShader(dxRenderObj.pixelShaderHDR.Get(), nullptr, 0);
			d3dCtx->PSSetConstantBuffers(0, 1, dxRenderObj.psConstantBuffer.GetAddressOf());
		}
		else {
			d3dCtx->PSSetShader(dxRenderObj.pixelShaderToneMap.Get(), nullptr, 0);
		}

		// Set input layout, vertex / index buffers.
		d3dCtx->IASetInputLayout(dxRenderObj.inputLayout.Get());
		d3dCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT strides = sizeof(VertexPositionTexcoord);
		UINT offsets = 0;
		d3dCtx->IASetVertexBuffers(0, 1, dxRenderObj.vertexBuffer.GetAddressOf(), &strides, &offsets);
		d3dCtx->IASetIndexBuffer(dxRenderObj.indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		// Draw quad.
		d3dCtx->DrawIndexed(6, 0, 0);

		ID3D11ShaderResourceView* nullrtv[] = { nullptr };
		d3dCtx->PSSetShaderResources(0, _countof(nullrtv), nullrtv);
	}
}