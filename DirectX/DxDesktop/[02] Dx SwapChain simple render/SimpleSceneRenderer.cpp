#include "SimpleSceneRenderer.h"
#include <Helpers/Dx/DxHelpers.h>
#include <Helpers/File.h>

#include <fstream>

#pragma comment(lib, "dxguid.lib")

namespace Tools {
	std::vector<uint8_t> ReadFile(const std::filesystem::path& filename) {
		std::vector<uint8_t> buffer;
		buffer.resize(std::filesystem::file_size(filename));
		
		std::ifstream inFile;
		inFile.open(filename, std::ios::binary);
		inFile.read((char*)buffer.data(), buffer.size());
		inFile.close();

		return buffer;
	}
}


namespace DxDesktop {
	SimpleSceneRenderer::SimpleSceneRenderer(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel)
		: swapChainPanel{ swapChainPanel }
		, loadingComplete{ false }
	{
		HRESULT hr = S_OK;
		{
			auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
			hr = dxDev->GetD2DFactory()->CreateDrawingStateBlock(&this->stateBlock);
			H::System::ThrowIfFailed(hr);
		}

		this->CreateDeviceDependentResources();
		this->CreateWindowSizeDependentResources();
	}

	SimpleSceneRenderer::~SimpleSceneRenderer()
	{}

	void SimpleSceneRenderer::CreateDeviceDependentResources() {
		HRESULT hr = S_OK;

		//// Load shaders.
		//auto fileDataVS = Tools::ReadFile(L"SampleVertexShader.cso");
		//auto fileDataPS = Tools::ReadFile(L"SamplePixelShader.cso");

		//// After the vertex shader file is loaded, create the shader and input layout.
		//auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();

		//hr = dxDev->GetD3DDevice()->CreateVertexShader(
		//	&fileDataVS[0],
		//	fileDataVS.size(),
		//	nullptr,
		//	&this->vertexShader
		//);
		//H::System::ThrowIfFailed(hr);

		//static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
		//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//	{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//};

		//hr = dxDev->GetD3DDevice()->CreateInputLayout(
		//	vertexDesc,
		//	ARRAYSIZE(vertexDesc),
		//	&fileDataVS[0],
		//	fileDataVS.size(),
		//	&this->inputLayout
		//);
		//H::System::ThrowIfFailed(hr);
		//

		//// After the pixel shader file is loaded, create the shader and constant buffer.
		//auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();

		//HRESULT hr = S_OK;
		//hr = dxDev->GetD3DDevice()->CreatePixelShader(
		//	&fileDataPS[0],
		//	fileDataPS.size(),
		//	nullptr,
		//	&this->pixelShader
		//);
		//H::System::ThrowIfFailed(hr);

		//CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		//hr = dxDev->GetD3DDevice()->CreateBuffer(
		//	&constantBufferDesc,
		//	nullptr,
		//	&this->constantBuffer
		//);
		//H::System::ThrowIfFailed(hr);


		//// Once both shaders are loaded, create the mesh.
		this->loadingComplete = true;

		//auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		//auto dxCtx = dxDev->LockContext();
		//auto d2dCtx = dxCtx->D2D();

		//// Create frame transformation effects
		//hr = d2dCtx->CreateEffect(CLSID_D2D1Scale, &this->scaleEffect);
		//H::System::ThrowIfFailed(hr);

		//hr = d2dCtx->CreateEffect(CLSID_D2D12DAffineTransform, &this->centerEffect);
		//H::System::ThrowIfFailed(hr);
	}

	void SimpleSceneRenderer::CreateWindowSizeDependentResources() {
	}

	void SimpleSceneRenderer::ReleaseDeviceDependentResources() {
		this->loadingComplete = false;
		this->vertexShader.Reset();
		this->inputLayout.Reset();
		this->pixelShader.Reset();
		this->constantBuffer.Reset();
		//this->vertexBuffer.Reset();
		//this->indexBuffer.Reset();
	}


	void SimpleSceneRenderer::Render() {
		// Loading is asynchronous. Only draw geometry after it's loaded.
		if (!this->loadingComplete) {
			return;
		}

		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto dxCtx = dxDev->LockContext();
		auto d2dCtx = dxCtx->D2D();

		d2dCtx->SaveDrawingState(this->stateBlock.Get());
		d2dCtx->BeginDraw();
		d2dCtx->Clear(D2D1::ColorF(D2D1::ColorF::Gray));


		// Ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
		// is lost. It will be handled during the next call to Present.
		HRESULT hr = d2dCtx->EndDraw();
		if (hr != D2DERR_RECREATE_TARGET) {
			H::System::ThrowIfFailed(hr);
		}

		d2dCtx->RestoreDrawingState(this->stateBlock.Get());
	}
}