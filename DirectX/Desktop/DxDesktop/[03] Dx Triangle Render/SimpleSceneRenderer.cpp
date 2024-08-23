#include "SimpleSceneRenderer.h"
#include <Helpers/Dx/DxHelpers.h>
#include <Helpers/Helpers.h>
#include <Helpers/File.h>


//#include <DirectXColors.h>
//#include <DirectXMath.h>
#include <fstream>

#pragma comment(lib, "dxguid.lib")

namespace {
	struct Vertex {
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 color;
	};
}

namespace Tools {
	std::vector<uint8_t> ReadFile(const std::filesystem::path& filename) {
		auto targetFile = H::ExePath() / filename;

		std::vector<uint8_t> buffer;
		buffer.resize(std::filesystem::file_size(targetFile));
		
		std::ifstream inFile;
		inFile.open(targetFile, std::ios::binary);
		inFile.read((char*)buffer.data(), buffer.size());
		inFile.close();

		return buffer;
	}
}

namespace Colors {
	XMGLOBALCONST DirectX::XMVECTORF32 Background = { { { 0.254901975f, 0.254901975f, 0.254901975f, 1.f } } }; // #414141
	XMGLOBALCONST DirectX::XMVECTORF32 Green = { { { 0.062745102f, 0.486274511f, 0.062745102f, 1.f } } }; // #107c10
	XMGLOBALCONST DirectX::XMVECTORF32 Blue = { { { 0.019607844f, 0.372549027f, 0.803921580f, 1.f } } }; // #055fcd
	XMGLOBALCONST DirectX::XMVECTORF32 Orange = { { { 0.764705896f, 0.176470593f, 0.019607844f, 1.f } } }; // #c32d05
	XMGLOBALCONST DirectX::XMVECTORF32 DarkGrey = { { { 0.200000003f, 0.200000003f, 0.200000003f, 1.f } } }; // #333333
	XMGLOBALCONST DirectX::XMVECTORF32 LightGrey = { { { 0.478431374f, 0.478431374f, 0.478431374f, 1.f } } }; // #7a7a7a
	XMGLOBALCONST DirectX::XMVECTORF32 OffWhite = { { { 0.635294139f, 0.635294139f, 0.635294139f, 1.f } } }; // #a2a2a2
	XMGLOBALCONST DirectX::XMVECTORF32 White = { { { 0.980392158f, 0.980392158f, 0.980392158f, 1.f } } }; // #fafafa
};

namespace ColorsLinear {
	XMGLOBALCONST DirectX::XMVECTORF32 Background = { { { 0.052860655f, 0.052860655f, 0.052860655f, 1.f } } };
	XMGLOBALCONST DirectX::XMVECTORF32 Green = { { { 0.005181516f, 0.201556236f, 0.005181516f, 1.f } } };
	XMGLOBALCONST DirectX::XMVECTORF32 Blue = { { { 0.001517635f, 0.114435382f, 0.610495627f, 1.f } } };
	XMGLOBALCONST DirectX::XMVECTORF32 Orange = { { { 0.545724571f, 0.026241219f, 0.001517635f, 1.f } } };
	XMGLOBALCONST DirectX::XMVECTORF32 DarkGrey = { { { 0.033104762f, 0.033104762f, 0.033104762f, 1.f } } };
	XMGLOBALCONST DirectX::XMVECTORF32 LightGrey = { { { 0.194617808f, 0.194617808f, 0.194617808f, 1.f } } };
	XMGLOBALCONST DirectX::XMVECTORF32 OffWhite = { { { 0.361306787f, 0.361306787f, 0.361306787f, 1.f } } };
	XMGLOBALCONST DirectX::XMVECTORF32 White = { { { 0.955973506f, 0.955973506f, 0.955973506f, 1.f } } };
};

namespace ColorsHDR {
	XMGLOBALCONST DirectX::XMVECTORF32 Background = { { { 0.052860655f * 2.f, 0.052860655f * 2.f, 0.052860655f * 2.f, 1.f } } };
	XMGLOBALCONST DirectX::XMVECTORF32 Green = { { { 0.005181516f * 2.f, 0.201556236f * 2.f, 0.005181516f * 2.f, 1.f } } };
	XMGLOBALCONST DirectX::XMVECTORF32 Blue = { { { 0.001517635f * 2.f, 0.114435382f * 2.f, 0.610495627f * 2.f, 1.f } } };
	XMGLOBALCONST DirectX::XMVECTORF32 Orange = { { { 0.545724571f * 2.f, 0.026241219f * 2.f, 0.001517635f * 2.f, 1.f } } };
	XMGLOBALCONST DirectX::XMVECTORF32 DarkGrey = { { { 0.033104762f * 2.f, 0.033104762f * 2.f, 0.033104762f * 2.f, 1.f } } };
	XMGLOBALCONST DirectX::XMVECTORF32 LightGrey = { { { 0.194617808f * 2.f, 0.194617808f * 2.f, 0.194617808f * 2.f, 1.f } } };
	XMGLOBALCONST DirectX::XMVECTORF32 OffWhite = { { { 0.361306787f * 2.f, 0.361306787f * 2.f, 0.361306787f * 2.f, 1.f } } };
	XMGLOBALCONST DirectX::XMVECTORF32 White = { { { 0.955973506f * 2.f, 0.955973506f * 2.f, 0.955973506f * 2.f, 1.f } } };
};



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

		// Load and create shaders.
		auto vertexShaderBlob = Tools::ReadFile(L"VertexShader.cso");

		hr = d3dDev->CreateVertexShader(
			vertexShaderBlob.data(),
			vertexShaderBlob.size(),
			nullptr,
			this->vertexShader.ReleaseAndGetAddressOf()
		);
		H::System::ThrowIfFailed(hr);

		
		auto pixelShaderBlob = Tools::ReadFile(L"PixelShader.cso");

		hr = d3dDev->CreatePixelShader(
			pixelShaderBlob.data(),
			pixelShaderBlob.size(),
			nullptr,
			this->pixelShader.ReleaseAndGetAddressOf()
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
			this->inputLayout.ReleaseAndGetAddressOf()
		);
		H::System::ThrowIfFailed(hr);

		// Create vertex buffer.
		static const Vertex vertexData[3] = {
			{ { -0.5f,  0.5f,  0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }, // Left-top / Red
			{ {  0.5f,  0.5f,  0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // Right-top / Green
			{ {  0.5f, -0.5f,  0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }, // Right-bottom / Blue
		};

		D3D11_SUBRESOURCE_DATA initialData = {};
		initialData.pSysMem = vertexData;

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(vertexData);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.StructureByteStride = sizeof(Vertex);

		hr = d3dDev->CreateBuffer(
			&bufferDesc,
			&initialData,
			this->vertexBuffer.ReleaseAndGetAddressOf()
		);
	}

	void SimpleSceneRenderer::CreateWindowSizeDependentResources() {
	}

	void SimpleSceneRenderer::ReleaseDeviceDependentResources() {
		this->inputLayout.Reset();
		this->vertexShader.Reset();
		this->vertexBuffer.Reset();
		this->pixelShader.Reset();
	}


	// Helper method to clear the back buffers.
	void SimpleSceneRenderer::Clear() {
		HRESULT hr = S_OK;

		// Clear the views.
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		auto depthStencilView = this->swapChainPanel->GetDepthStencilView();
		auto renderTargetView1 = this->swapChainPanel->GetRenderTargetView();
		
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
		hr = renderTargetView1.As(&renderTargetView);
		H::System::ThrowIfFailed(hr);

		// Use linear clear color for gamma-correct rendering.
		d3dCtx->ClearRenderTargetView(renderTargetView.Get(), Colors::LightGrey);
		
		d3dCtx->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		d3dCtx->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

		// Set the viewport.
		auto viewport = this->swapChainPanel->GetScreenViewport();
		d3dCtx->RSSetViewports(1, &viewport);
	}

	void SimpleSceneRenderer::Render() {
		this->Clear();

		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		// Set input assembler state.
		d3dCtx->IASetInputLayout(this->inputLayout.Get());

		UINT strides = sizeof(Vertex);
		UINT offsets = 0;
		d3dCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		d3dCtx->IASetVertexBuffers(0, 1, this->vertexBuffer.GetAddressOf(), &strides, &offsets);

		// Set shaders.
		d3dCtx->VSSetShader(this->vertexShader.Get(), nullptr, 0);
		d3dCtx->GSSetShader(nullptr, nullptr, 0);
		d3dCtx->PSSetShader(this->pixelShader.Get(), nullptr, 0);

		// Draw triangle.
		d3dCtx->Draw(3, 0);
	}
}