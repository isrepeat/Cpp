#include "SceneShaderEffects.h"
#include <Helpers/Dx/DxHelpers.h>
#include <Helpers/Helpers.h>
#include <Helpers/FileSystem.h>

#include "Tools.h"
#include <DirectXMath.h>
#include <fstream>

#pragma comment(lib, "dxguid.lib")

const std::filesystem::path g_ProjectRootNamespace = L"" PP_STRINGIFY(MSBuildProject__RootNamespace);


namespace DxSamples {
	SceneShaderEffects::SceneShaderEffects(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel)
		: swapChainPanel{ swapChainPanel }
		, dxLinkageGraphPipeline{ this->swapChainPanel->GetDxDevice() }
		, renderPipeline{ this->swapChainPanel }
	{
		HRESULT hr = S_OK;

		this->CreateDeviceDependentResources();
		this->CreateWindowSizeDependentResources();
	}

	SceneShaderEffects::~SceneShaderEffects()
	{}

	// Executes basic render loop.
	void SceneShaderEffects::Render() {
		this->renderLoopTimer.Tick([this] {
			this->Update();
			});

		this->RenderInternal();
	}

	void SceneShaderEffects::OnWindowSizeChanged(H::Size newSize) {
		this->swapChainPanel->SetLogicalSize(static_cast<H::Size_f>(newSize));
		this->CreateWindowSizeDependentResources();
	}


	void SceneShaderEffects::CreateDeviceDependentResources() {
		HRESULT hr = S_OK;

		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDev = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d2dCtx = dxCtx->D2D();

		DxRenderObjImage::Params dxRenderObjImageParams;
		dxRenderObjImageParams.imagePath = L"Textures\\Dune_screenshot_24s.png";
		dxRenderObjImageParams.vertexShader = g_ProjectRootNamespace / L"Image_VS.cso";
		dxRenderObjImageParams.pixelShader = g_ProjectRootNamespace / L"Image_PS.cso";
		this->dxRenderObjImage = std::make_unique<DxRenderObjImage>(this->swapChainPanel, dxRenderObjImageParams);


		auto vertexShaderGraph = this->dxLinkageGraphPipeline.GetVertexShaderGraph();

		this->dxLinkageGraphPipeline.SetInputSignature();
		auto vertexShaderInputNode = this->dxLinkageGraphPipeline.GetVertexShaderInputNode();

		auto vsModule1 = this->dxLinkageGraphPipeline.AddModule("Function1.VS.hlsl");
		vsModule1.shaderLibraryInstance->BindResource(0, 0, 1);
		vsModule1.shaderLibraryInstance->BindSampler(0, 0, 1);

		this->dxLinkageGraphPipeline.CallFunction(vsModule1, "VertexFunction");
		hr = vertexShaderGraph->PassValue(vertexShaderInputNode.Get(), 0, vsModule1.shaderCallFunctionNode.Get(), 0);
		H::System::ThrowIfFailed(hr);
		hr = vertexShaderGraph->PassValue(vertexShaderInputNode.Get(), 1, vsModule1.shaderCallFunctionNode.Get(), 1);
		H::System::ThrowIfFailed(hr);

		auto vsModule2 = this->dxLinkageGraphPipeline.AddModule("Function2.VS.hlsl");
		vsModule2.shaderLibraryInstance->BindResource(0, 0, 1);
		vsModule2.shaderLibraryInstance->BindSampler(0, 0, 1);

		this->dxLinkageGraphPipeline.CallFunction(vsModule2, "VertexFunctionNew");
		hr = vertexShaderGraph->PassValue(vsModule1.shaderCallFunctionNode.Get(), 0, vsModule2.shaderCallFunctionNode.Get(), 0);
		H::System::ThrowIfFailed(hr);
		hr = vertexShaderGraph->PassValue(vsModule1.shaderCallFunctionNode.Get(), 1, vsModule2.shaderCallFunctionNode.Get(), 1);
		H::System::ThrowIfFailed(hr);

		this->dxLinkageGraphPipeline.SetOutputSignature();
		auto vertexShaderOutputNode = this->dxLinkageGraphPipeline.GetVertexShaderOutputNode();

		hr = vertexShaderGraph->PassValue(vsModule2.shaderCallFunctionNode.Get(), 0, vertexShaderOutputNode.Get(), 0);
		H::System::ThrowIfFailed(hr);
		hr = vertexShaderGraph->PassValue(vsModule2.shaderCallFunctionNode.Get(), 1, vertexShaderOutputNode.Get(), 1);
		H::System::ThrowIfFailed(hr);

		this->dxLinkageGraphPipeline.LogVertexShaderGraph();
		this->dxLinkageGraphPipeline.CreateVertexShader();

	}

	void SceneShaderEffects::CreateWindowSizeDependentResources() {
		this->dxRenderObjImage->CreateWindowSizeDependentResources();
	}

	void SceneShaderEffects::ReleaseDeviceDependentResources() {
		this->dxRenderObjImage->ReleaseDeviceDependentResources();
	}

	void SceneShaderEffects::Update() {
	}

	void SceneShaderEffects::Clear() {
		HRESULT hr = S_OK;

		// Clear the views.
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		auto renderTargetView = this->swapChainPanel->GetRenderTargetView();
		d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Gray);

		ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
		d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

		auto viewport = this->swapChainPanel->GetScreenViewport();
		d3dCtx->RSSetViewports(1, &viewport);
	}

	void SceneShaderEffects::RenderInternal() {
		// Don't try to render anything before the first Update.
		if (this->renderLoopTimer.GetFrameCount() == 0) {
			return;
		}

		this->Clear();
		this->RenderScene();
	}

	void SceneShaderEffects::RenderScene() {
		HRESULT hr = S_OK;
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDevice = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		//
		// Render dxRenderObjImage texture to swapChain RTV.
		//
		{
			auto& dxRenderObj = this->dxRenderObjImage;
			dxRenderObj->UpdateBuffers();

			this->renderPipeline.SetTexture(dxRenderObj->GetObj()->textureSRV);

			this->renderPipeline.SetInputLayout(
				//this->inputLayout
				this->dxLinkageGraphPipeline.GetInputLayout()
			);

			this->renderPipeline.SetVertexShader(
				//this->vertexShader
				this->dxLinkageGraphPipeline.GetVertexShader()
			);

			//this->renderPipeline.SetVertexShader(
			//	dxRenderObj->GetObj()->vertexShader,
			//	dxRenderObj->GetObj()->vsConstantBuffer
			//);

			this->renderPipeline.SetPixelShader(
				dxRenderObj->GetObj()->pixelShader,
				dxRenderObj->GetObj()->psConstantBuffer
			);

			this->renderPipeline.Draw();
		}


		ID3D11ShaderResourceView* nullrtv[] = { nullptr };
		d3dCtx->PSSetShaderResources(0, _countof(nullrtv), nullrtv);

		// ...
		// swapChain proxy texture (16:16:16:16) to swapChain RTV (10:10:10:2).
	}
}